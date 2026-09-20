#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <vector>

// Deterministic model ticks, not measurements of this machine.
using Tick = std::uint64_t;
constexpr std::size_t max_jobs = 256;

Tick checked_add(Tick a, Tick b) {
    if (b > std::numeric_limits<Tick>::max() - a) {
        throw std::overflow_error("model clock overflow");
    }
    return a + b;
}

struct Job {
    Tick arrival;
    Tick service;
    Tick budget;
};

enum class Outcome { pending, completed, rejected, timed_out };

struct Record {
    Job job;
    Tick deadline;
    Outcome outcome = Outcome::pending;
    Tick terminal = 0;
    std::optional<Tick> start;
    std::optional<Tick> work_done;
};

std::vector<Record> simulate(const std::vector<Job>& jobs,
                             std::size_t waiting_capacity) {
    if (jobs.size() > max_jobs || waiting_capacity > max_jobs) {
        throw std::invalid_argument("model size limit");
    }
    std::vector<Record> records;
    records.reserve(jobs.size());
    for (std::size_t i = 0; i < jobs.size(); ++i) {
        const auto& j = jobs[i];
        if (j.service == 0 || (i != 0 && j.arrival < jobs[i - 1].arrival)) {
            throw std::invalid_argument("positive service and sorted arrivals required");
        }
        records.push_back({j, checked_add(j.arrival, j.budget),
                           Outcome::pending, 0, std::nullopt, std::nullopt});
    }

    std::deque<std::size_t> waiting;
    std::optional<std::size_t> active;
    Tick finish = 0;
    std::size_t next_arrival = 0;
    auto start = [&](std::size_t id, Tick now) {
        finish = checked_add(now, records[id].job.service);
        records[id].start = now;
        active = id;
    };
    auto expire = [&](Record& r, Tick now) {
        r.outcome = Outcome::timed_out;
        r.terminal = now;
    };

    while (next_arrival < records.size() || active || !waiting.empty()) {
        std::optional<Tick> next;
        auto consider = [&](Tick t) {
            if (!next || t < *next) next = t;
        };
        if (next_arrival < records.size()) {
            consider(records[next_arrival].job.arrival);
        }
        if (active) {
            consider(finish);
            if (records[*active].outcome == Outcome::pending) {
                consider(records[*active].deadline);
            }
        }
        for (auto id : waiting) consider(records[id].deadline);
        assert(next);
        const Tick now = *next;

        // Tie order: finish, expire, start older waiting work, then arrivals.
        // A completion exactly at its deadline succeeds.
        if (active && finish == now) {
            auto& r = records[*active];
            r.work_done = now;
            if (r.outcome == Outcome::pending) {
                r.outcome = Outcome::completed;
                r.terminal = now;
            }
            active.reset();
        }
        if (active && records[*active].outcome == Outcome::pending &&
            records[*active].deadline <= now) {
            expire(records[*active], now);
        }
        for (auto it = waiting.begin(); it != waiting.end();) {
            auto& r = records[*it];
            if (r.deadline <= now) {
                expire(r, now);
                it = waiting.erase(it);
            } else {
                ++it;
            }
        }
        if (!active && !waiting.empty()) {
            const auto id = waiting.front();
            waiting.pop_front();
            start(id, now);
        }
        while (next_arrival < records.size() &&
               records[next_arrival].job.arrival == now) {
            const auto id = next_arrival++;
            auto& r = records[id];
            if (r.deadline == now) {
                expire(r, now);
            } else if (!active) {
                start(id, now);
            } else if (waiting.size() < waiting_capacity) {
                waiting.push_back(id);
            } else {
                r.outcome = Outcome::rejected;
                r.terminal = now;
            }
        }
        assert(waiting.size() <= waiting_capacity);
    }
    for (const auto& r : records) {
        assert(r.outcome != Outcome::pending);
        assert(r.terminal >= r.job.arrival);
        assert(r.start.has_value() == r.work_done.has_value());
    }
    return records;
}

std::optional<Tick> percentile(std::vector<Tick> values, unsigned percent) {
    if (percent == 0 || percent > 100 || values.size() > max_jobs) {
        throw std::invalid_argument("percentile input limit");
    }
    if (values.empty()) return std::nullopt;
    std::sort(values.begin(), values.end());
    // Nearest rank ceil(percent * N / 100); N <= 256 prevents overflow.
    const auto rank = (static_cast<std::size_t>(percent) * values.size() + 99) / 100;
    return values[rank - 1];
}

void report(std::string_view name, const std::vector<Record>& records) {
    std::size_t rejected = 0;
    std::size_t timed_out = 0;
    std::vector<Tick> success_latency;
    std::cout << name << " (MODEL ticks)\n";
    for (std::size_t i = 0; i < records.size(); ++i) {
        const auto& r = records[i];
        std::string_view status;
        switch (r.outcome) {
        case Outcome::completed:
            status = "completed";
            success_latency.push_back(r.terminal - r.job.arrival);
            break;
        case Outcome::rejected: status = "rejected"; ++rejected; break;
        case Outcome::timed_out: status = "timed_out"; ++timed_out; break;
        case Outcome::pending: throw std::logic_error("unfinished simulation");
        }
        const Tick wait = r.start.value_or(r.terminal) - r.job.arrival;
        const Tick work = r.start ? *r.work_done - *r.start : 0;
        std::cout << "  id=" << i << " " << status << " queue=" << wait
                  << " service_work=" << work
                  << " e2e_to_outcome=" << r.terminal - r.job.arrival << '\n';
    }
    assert(success_latency.size() + rejected + timed_out == records.size());
    std::cout << "  offered=" << records.size() << " completed=" << success_latency.size()
              << " rejected=" << rejected << " timed_out=" << timed_out;
    if (auto p50 = percentile(success_latency, 50)) {
        std::cout << " success_only_N=" << success_latency.size() << " p50=" << *p50
                  << " p95=" << *percentile(success_latency, 95);
    } else {
        std::cout << " success_only_N=0 quantiles=NA";
    }
    std::cout << '\n';
}

void test_oracle() {
    // Independent FIFO recurrence: no expiry/rejection in this input domain.
    for (Tick gap = 0; gap < 4; ++gap) {
        std::vector<Job> jobs;
        for (Tick i = 0; i < 16; ++i) jobs.push_back({i * gap, 1 + i % 5, 200});
        const auto records = simulate(jobs, jobs.size());
        Tick previous_finish = 0;
        for (std::size_t i = 0; i < jobs.size(); ++i) {
            const Tick expected_start = std::max(jobs[i].arrival, previous_finish);
            previous_finish = checked_add(expected_start, jobs[i].service);
            assert(records[i].outcome == Outcome::completed);
            assert(records[i].start == expected_start);
            assert(records[i].work_done == previous_finish);
            assert(records[i].terminal == previous_finish);
        }
    }
}

void test_boundaries() {
    assert(simulate({}, 0).empty());
    assert(!percentile({}, 99));
    assert(percentile({5, 1, 3, 2}, 50) == 2);
    assert(percentile({5, 1, 3, 2}, 95) == 5);
    const auto tie = simulate({{0, 2, 2}, {2, 1, 1}}, 0);
    assert(tie[0].outcome == Outcome::completed && tie[1].outcome == Outcome::completed);
    const auto zero_capacity = simulate({{0, 2, 10}, {1, 1, 10}}, 0);
    assert(zero_capacity[1].outcome == Outcome::rejected);
    const auto one_capacity = simulate({{0, 1, 10}, {0, 1, 10}, {0, 1, 10}}, 1);
    assert(one_capacity[1].start == 1 && one_capacity[2].outcome == Outcome::rejected);
    const auto zero_budget = simulate({{0, 1, 0}}, 0);
    assert(zero_budget[0].outcome == Outcome::timed_out && !zero_budget[0].start);
    const auto recovery = simulate({{0, 5, 20}, {0, 1, 20}, {0, 1, 20}, {20, 1, 10}}, 1);
    assert(recovery[2].outcome == Outcome::rejected);
    assert(recovery[3].start == 20 && recovery[3].terminal == 21);
    auto must_throw = [](auto operation) {
        bool threw = false;
        try { operation(); } catch (const std::invalid_argument&) { threw = true; }
        catch (const std::overflow_error&) { threw = true; }
        assert(threw);
    };
    must_throw([] { (void)simulate({{1, 1, 1}, {0, 1, 1}}, 1); });
    must_throw([] { (void)simulate({{0, 0, 1}}, 1); });
    must_throw([] { (void)simulate({}, max_jobs + 1); });
    must_throw([] { (void)simulate(std::vector<Job>(max_jobs + 1, {0, 1, 1}), 1); });
    must_throw([] { (void)percentile({1}, 0); });
    const Tick maximum = std::numeric_limits<Tick>::max();
    must_throw([=] { (void)simulate({{maximum, 1, 1}}, 0); });
    must_throw([=] { (void)simulate({{maximum - 1, 2, 1}}, 0); });
    const auto maximum_finish = simulate({{maximum - 1, 1, 1}}, 0);
    assert(maximum_finish[0].terminal == maximum);
}

int main() {
    test_oracle();
    test_boundaries();
    std::vector<Job> open;
    std::vector<Job> closed;
    Tick next = 0;
    for (Tick i = 0; i < 8; ++i) {
        const Tick service = i == 0 ? 5 : 1;
        open.push_back({i, service, 20});
        closed.push_back({next, service, 20});
        next = checked_add(next, service); // One serial client, zero think time.
    }
    const auto open_result = simulate(open, 2);
    const auto closed_result = simulate(closed, 2);
    assert(open_result[3].outcome == Outcome::rejected);
    assert(open_result[4].outcome == Outcome::rejected);
    for (const auto& r : closed_result) assert(r.outcome == Outcome::completed);
    report("open arrival=0..7", open_result);
    report("closed one-client", closed_result);
    const auto deadlines = simulate({{0, 5, 2}, {0, 1, 4}, {1, 1, 10},
                                     {2, 1, 10}, {4, 1, 2}}, 2);
    assert(deadlines[0].terminal == 2 && deadlines[0].work_done == 5);
    assert(deadlines[1].terminal == 4 && !deadlines[1].start);
    assert(deadlines[2].outcome == Outcome::completed && deadlines[2].terminal == 6);
    assert(deadlines[3].outcome == Outcome::rejected);
    assert(deadlines[4].outcome == Outcome::timed_out && !deadlines[4].start);
    report("deadline/non-preemptive", deadlines);
    std::cout << "oracle_cases=64; boundary checks passed\n";
}
