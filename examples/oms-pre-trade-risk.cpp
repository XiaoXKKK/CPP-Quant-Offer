#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>

// Fictional buy-only protocol. Integer cost units; no fees, sells or busts.
// Reports carry authoritative cumulative quantity/cost and per-order monotonic IDs.
using Units = std::uint64_t;
constexpr Units max_units = std::numeric_limits<Units>::max();
constexpr std::size_t order_capacity = 4;
constexpr std::size_t report_capacity = 16;
enum class State { prepared, pending_new, live, pending_cancel, unknown,
                   filled, canceled, rejected, local_failed };
enum class Kind { ack, trade, cancel_ack, cancel_reject, reject_new };
enum class Result { ok, duplicate, stale, foreign, bad_state, invalid,
                    risk_limit, order_full, report_full, conflict, halted };

bool terminal(State s) {
    return s == State::filled || s == State::canceled ||
           s == State::rejected || s == State::local_failed;
}
struct Order {
    Units id = 0;
    Units quantity = 0;
    Units limit_price = 0;
    Units filled = 0;
    Units cost = 0;
    Units reserved = 0;
    Units last_report = 0;
    bool cancel_requested = false;
    State state = State::prepared;
};
struct Report {
    Units session;
    Units order_id;
    Units report_id;
    Kind kind;
    Units cumulative_quantity;
    Units cumulative_cost;
    bool operator==(const Report&) const = default;
};

class Oms {
public:
    Oms(Units session, Units limit) : session_(session), limit_(limit) {
        assert(session != 0);
    }

    Result prepare(Units id, Units quantity, Units limit_price) {
        if (halted_) return Result::halted;
        if (id == 0 || quantity == 0 || limit_price == 0 ||
            quantity > max_units / limit_price) return Result::invalid;
        if (find(id)) return Result::conflict; // IDs are never reused in this instance.
        if (count_ == order_capacity) return Result::order_full;
        const Units needed = quantity * limit_price;
        if (needed > limit_ - spent_ - reserved_) return Result::risk_limit;
        orders_[count_++] = {id, quantity, limit_price, 0, 0, needed, 0, false, State::prepared};
        reserved_ += needed;
        verify();
        return Result::ok;
    }

    Result fail_before_send(Units id) {
        auto* order = find(id);
        if (!order || order->state != State::prepared) return Result::bad_state;
        reserved_ -= order->reserved;
        order->reserved = 0;
        order->state = State::local_failed;
        verify();
        return Result::ok;
    }

    // Call before handing any bytes to transport; retry dispatch is prohibited.
    Result dispatch(Units id) {
        if (halted_) return Result::halted;
        auto* order = find(id);
        if (!order || order->state != State::prepared) return Result::bad_state;
        order->state = State::pending_new;
        return Result::ok;
    }

    Result uncertain(Units id) { // Partial send, connection failure, or response timeout.
        auto* order = find(id);
        if (!order || order->state == State::prepared || terminal(order->state)) {
            return Result::bad_state;
        }
        order->state = State::unknown;
        verify();
        return Result::ok;
    }

    Result request_cancel(Units id) {
        if (halted_) return Result::halted;
        auto* order = find(id);
        if (!order || (order->state != State::pending_new && order->state != State::live)) {
            return Result::bad_state;
        }
        order->cancel_requested = true;
        order->state = State::pending_cancel; // No reservation is released here.
        verify();
        return Result::ok;
    }

    Result on_report(const Report& report) {
        auto* order = find(report.order_id);
        if (report.session != session_ || !order) return Result::foreign;
        if (report.report_id == 0) return stop(*order, Result::invalid);
        switch (report.kind) {
        case Kind::ack:
        case Kind::trade:
        case Kind::cancel_ack:
        case Kind::cancel_reject:
        case Kind::reject_new:
            break;
        default:
            return stop(*order, Result::invalid);
        }
        for (std::size_t i = 0; i < reports_; ++i) {
            const auto& seen = history_[i];
            if (seen.order_id == report.order_id && seen.report_id == report.report_id) {
                if (seen == report) return Result::duplicate;
                return stop(*order, Result::conflict);
            }
        }
        if (report.report_id < order->last_report) return Result::stale;
        if (reports_ == report_capacity) return stop(*order, Result::report_full);
        if (order->state == State::prepared || order->state == State::local_failed ||
            report.cumulative_quantity < order->filled ||
            report.cumulative_quantity > order->quantity ||
            report.cumulative_cost < order->cost) return stop(*order, Result::invalid);

        const Units new_quantity = report.cumulative_quantity - order->filled;
        const Units new_cost = report.cumulative_cost - order->cost;
        // new_quantity <= original quantity, whose price product was checked.
        if (new_cost > new_quantity * order->limit_price ||
            (new_quantity != 0 && new_cost == 0) ||
            (terminal(order->state) && new_quantity != 0)) return stop(*order, Result::invalid);
        if (report.kind == Kind::trade && new_quantity == 0) return stop(*order, Result::invalid);
        if ((report.kind == Kind::cancel_ack || report.kind == Kind::cancel_reject) &&
            !order->cancel_requested) return stop(*order, Result::invalid);
        if (report.kind == Kind::reject_new &&
            (report.cumulative_quantity != 0 || terminal(order->state))) {
            return stop(*order, Result::invalid);
        }

        Order next = *order;
        next.filled = report.cumulative_quantity;
        next.cost = report.cumulative_cost;
        next.last_report = report.report_id;
        if (!terminal(next.state)) {
            if (report.kind == Kind::reject_new) {
                next.state = State::rejected;
            } else if (next.filled == next.quantity) {
                next.state = State::filled;
            } else if (report.kind == Kind::cancel_ack) {
                next.state = State::canceled;
            } else if (report.kind == Kind::cancel_reject) {
                next.cancel_requested = false;
                if (next.state != State::unknown) next.state = State::live;
            } else if (next.state == State::pending_new) {
                next.state = State::live;
            }
        }
        next.reserved = terminal(next.state) ? 0 : (next.quantity - next.filled) * next.limit_price;
        // Each applied report moves at most its old reservation into spent.
        if (new_cost > order->reserved || next.reserved > order->reserved - new_cost) {
            return stop(*order, Result::invalid);
        }
        const Units remaining_reserved = reserved_ - order->reserved + next.reserved;
        if (new_cost > limit_ - spent_ || remaining_reserved > limit_ - spent_ - new_cost) {
            return stop(*order, Result::invalid);
        }
        spent_ += new_cost;
        reserved_ = remaining_reserved;
        *order = next;
        history_[reports_++] = report;
        verify();
        return Result::ok;
    }

    std::optional<Order> read(Units id) const {
        for (std::size_t i = 0; i < count_; ++i) if (orders_[i].id == id) return orders_[i];
        return std::nullopt;
    }
    Units spent() const { return spent_; }
    Units reserved() const { return reserved_; }
    bool halted() const { return halted_; }

private:
    Order* find(Units id) {
        for (std::size_t i = 0; i < count_; ++i) if (orders_[i].id == id) return &orders_[i];
        return nullptr;
    }
    Result stop(Order& order, Result reason) {
        halted_ = true; // New admission/dispatch/cancel stop; reconcile externally.
        if (!terminal(order.state) && order.state != State::prepared) order.state = State::unknown;
        verify();
        return reason;
    }
    void verify() const {
        Units reservation_sum = 0;
        Units spent_sum = 0;
        for (std::size_t i = 0; i < count_; ++i) {
            const auto& order = orders_[i];
            assert(order.filled <= order.quantity);
            const Units expected = terminal(order.state) ? 0 :
                (order.quantity - order.filled) * order.limit_price;
            assert(order.reserved == expected);
            assert(order.reserved <= max_units - reservation_sum);
            assert(order.cost <= max_units - spent_sum);
            reservation_sum += order.reserved;
            spent_sum += order.cost;
        }
        assert(reservation_sum == reserved_ && spent_sum == spent_);
        assert(spent_ <= limit_ && reserved_ <= limit_ - spent_);
    }
    Units session_;
    Units limit_;
    Units spent_ = 0;
    Units reserved_ = 0;
    bool halted_ = false;
    std::array<Order, order_capacity> orders_{};
    std::array<Report, report_capacity> history_{};
    std::size_t count_ = 0;
    std::size_t reports_ = 0;
};

void test_hand_trajectory() {
    Oms oms(7, 1000);
    assert(oms.prepare(1, 10, 10) == Result::ok);
    assert(oms.reserved() == 100 && oms.spent() == 0);
    assert(oms.dispatch(1) == Result::ok);
    const Report fill{7, 1, 2, Kind::trade, 3, 27}; // Fill arrives before ack.
    assert(oms.on_report(fill) == Result::ok);
    assert(oms.read(1)->state == State::live);
    assert(oms.spent() == 27 && oms.reserved() == 70);
    assert(oms.on_report(fill) == Result::duplicate);
    assert(oms.on_report({7, 1, 1, Kind::ack, 0, 0}) == Result::stale);
    assert(oms.request_cancel(1) == Result::ok);
    assert(oms.on_report({7, 1, 3, Kind::ack, 3, 27}) == Result::ok);
    assert(oms.read(1)->state == State::pending_cancel);
    assert(oms.on_report({7, 1, 4, Kind::trade, 5, 45}) == Result::ok);
    assert(oms.spent() == 45 && oms.reserved() == 50);
    assert(oms.uncertain(1) == Result::ok);
    assert(oms.read(1)->state == State::unknown && oms.reserved() == 50);
    assert(oms.dispatch(1) == Result::bad_state);
    // Authoritative cancel outcome includes a sixth fill not separately received.
    const Report canceled{7, 1, 6, Kind::cancel_ack, 6, 54};
    assert(oms.on_report(canceled) == Result::ok);
    assert(oms.read(1)->state == State::canceled);
    assert(oms.read(1)->filled == 6 && oms.spent() == 54 && oms.reserved() == 0);
    assert(oms.on_report({7, 1, 5, Kind::trade, 6, 54}) == Result::stale);
    assert(oms.on_report(canceled) == Result::duplicate);
    assert(oms.prepare(2, 10, 10) == Result::ok);
    assert(oms.fail_before_send(2) == Result::ok);
    assert(oms.reserved() == 0 && oms.spent() == 54);
    assert(oms.prepare(3, 10, 10) == Result::ok);
    assert(oms.dispatch(3) == Result::ok);
    assert(oms.on_report({7, 3, 1, Kind::reject_new, 0, 0}) == Result::ok);
    assert(oms.reserved() == 0 && oms.spent() == 54);
    std::cout << "hand trajectory: filled=6 canceled_remainder=4 spent=54 reserved=0\n";
}

void test_races_and_unknown() {
    Oms oms(7, 100);
    assert(oms.prepare(1, 10, 10) == Result::ok);
    assert(oms.prepare(2, 1, 1) == Result::risk_limit);
    assert(oms.dispatch(1) == Result::ok);
    assert(oms.request_cancel(1) == Result::ok); // Cancel before new-order ack.
    assert(oms.on_report({7, 1, 1, Kind::trade, 10, 90}) == Result::ok);
    assert(oms.read(1)->state == State::filled && oms.spent() == 90 && oms.reserved() == 0);
    assert(oms.on_report({7, 1, 2, Kind::cancel_reject, 10, 90}) == Result::ok);
    assert(oms.read(1)->state == State::filled && oms.reserved() == 0);

    Oms unknown(7, 100);
    assert(unknown.prepare(1, 5, 10) == Result::ok);
    assert(unknown.dispatch(1) == Result::ok);
    assert(unknown.uncertain(1) == Result::ok);
    assert(unknown.fail_before_send(1) == Result::bad_state);
    assert(unknown.on_report({7, 1, 1, Kind::ack, 0, 0}) == Result::ok);
    assert(unknown.read(1)->state == State::unknown && unknown.reserved() == 50);
    assert(unknown.on_report({7, 1, 2, Kind::trade, 2, 18}) == Result::ok);
    assert(unknown.read(1)->state == State::unknown && unknown.reserved() == 30);
    assert(unknown.on_report({7, 1, 3, Kind::trade, 5, 45}) == Result::ok);
    assert(unknown.read(1)->state == State::filled && unknown.spent() == 45);

    Oms cancel_rejected(7, 100);
    assert(cancel_rejected.prepare(1, 5, 10) == Result::ok);
    assert(cancel_rejected.dispatch(1) == Result::ok);
    assert(cancel_rejected.request_cancel(1) == Result::ok);
    assert(cancel_rejected.on_report({7, 1, 1, Kind::cancel_reject, 1, 9}) == Result::ok);
    assert(cancel_rejected.read(1)->state == State::live);
    assert(cancel_rejected.reserved() == 40 && cancel_rejected.spent() == 9);
    std::cout << "fill/cancel races and unknown retention passed\n";
}

void test_limits_and_failures() {
    Oms max(7, max_units);
    assert(max.prepare(1, max_units, 2) == Result::invalid);
    assert(max.prepare(1, 0, 1) == Result::invalid);
    assert(max.prepare(1, max_units, 1) == Result::ok);
    assert(max.dispatch(1) == Result::ok);
    assert(max.on_report({7, 1, max_units, Kind::trade, max_units, max_units}) == Result::ok);
    assert(max.spent() == max_units && max.reserved() == 0);

    Oms bounded(7, 100);
    for (Units id = 1; id <= order_capacity; ++id) {
        assert(bounded.prepare(id, 1, 1) == Result::ok);
        assert(bounded.fail_before_send(id) == Result::ok);
    }
    assert(bounded.prepare(5, 1, 1) == Result::order_full);
    assert(bounded.prepare(1, 1, 1) == Result::conflict);

    Oms history(7, 100);
    assert(history.prepare(1, 5, 10) == Result::ok);
    assert(history.dispatch(1) == Result::ok);
    for (Units id = 1; id <= report_capacity; ++id) {
        assert(history.on_report({7, 1, id, Kind::ack, 0, 0}) == Result::ok);
    }
    assert(history.on_report({7, 1, 17, Kind::ack, 0, 0}) == Result::report_full);
    assert(history.halted() && history.reserved() == 50 && history.spent() == 0);
    assert(history.prepare(2, 1, 1) == Result::halted);

    Oms conflict(7, 100);
    assert(conflict.prepare(1, 5, 10) == Result::ok);
    assert(conflict.dispatch(1) == Result::ok);
    assert(conflict.on_report({8, 1, 1, Kind::trade, 1, 9}) == Result::foreign);
    const Report accepted{7, 1, 1, Kind::trade, 1, 9};
    assert(conflict.on_report(accepted) == Result::ok);
    assert(conflict.on_report({7, 1, 1, Kind::trade, 2, 18}) == Result::conflict);
    assert(conflict.halted() && conflict.spent() == 9 && conflict.reserved() == 40);

    for (const auto& bad : std::array<Report, 5>{{
             {7, 1, 1, Kind::trade, 6, 54}, {7, 1, 1, Kind::trade, 1, 11},
             {7, 1, 1, Kind::trade, 0, 0}, {7, 1, 1, Kind::cancel_ack, 0, 0},
             {7, 1, 1, static_cast<Kind>(99), 1, 9}}}) {
        Oms invalid(7, 100);
        assert(invalid.prepare(1, 5, 10) == Result::ok);
        assert(invalid.dispatch(1) == Result::ok);
        assert(invalid.on_report(bad) == Result::invalid);
        assert(invalid.halted() && invalid.spent() == 0 && invalid.reserved() == 50);
        assert(invalid.read(1)->state == State::unknown);
    }
    std::cout << "capacity, duplicate conflict, foreign session and integer boundaries passed\n";
}

int main() {
    test_hand_trajectory();
    test_races_and_unknown();
    test_limits_and_failures();
    std::cout << "fictional single-owner OMS; no FIX transport or regulatory model\n";
}
