import { shuffle } from '../lib/study.mjs';
import { loadProgress, saveProgress } from './progress';
type Question = {
  id: string;
  level: string;
  prompt: string;
  answer: string;
  rubric: string[];
  source: {
    kind: string;
    url?: string;
    title?: string;
    accessed?: string;
    published?: string | null;
    interviewDate?: string | null;
    note?: string;
  };
  topicId: string;
  title: string;
};
type Topic = {
  id: string;
  title: string;
  roles: string[];
  companyTypes: string[];
  questions: Question[];
};
const topics: Topic[] = JSON.parse(document.querySelector('#interview-data')!.textContent!);
const el = <T extends HTMLElement>(id: string) => document.getElementById(id) as T;
const topicFilter = el<HTMLSelectElement>('interview-topic');
const levelFilter = el<HTMLSelectElement>('interview-level');
const roleFilter = el<HTMLSelectElement>('interview-role');
const companyFilter = el<HTMLSelectElement>('interview-company-type');
const card = el('interview-card');
const draft = el<HTMLTextAreaElement>('interview-draft');
const next = el<HTMLButtonElement>('next-question');
let deck: Question[] = [];
let position = 0;
let current: Question | undefined;
const base = import.meta.env.BASE_URL.replace(/\/$/, '');
function display() {
  current = deck[position];
  if (!current) {
    card.hidden = true;
    el('interview-status').textContent = '当前范围没有题目，请调整筛选。';
    return;
  }
  card.hidden = false;
  el('interview-intro').hidden = true;
  el('question-level').textContent = current.level;
  const link = el<HTMLAnchorElement>('question-topic');
  link.textContent = current.title;
  link.href = `${base}/topics/${current.topicId}/#${current.id}`;
  el('question-prompt').textContent = current.prompt;
  draft.value = loadProgress().drafts[current.id] || '';
  el<HTMLDetailsElement>('interview-answer').open = false;
  el('question-answer').textContent = current.answer;
  el('question-rubric').replaceChildren(
    ...current.rubric.map((point) => {
      const item = document.createElement('li');
      item.textContent = point;
      return item;
    }),
  );
  const source = el('question-source');
  source.replaceChildren();
  if (current.source.kind === 'public-interview') {
    const a = document.createElement('a');
    a.href = current.source.url!;
    a.textContent = current.source.title!;
    a.target = '_blank';
    a.rel = 'noreferrer';
    source.append(
      a,
      ` · 公开面经 · 访问 ${current.source.accessed} · 发布 ${current.source.published || '未知'} · 面试 ${current.source.interviewDate || '未知'} · ${current.source.note}`,
    );
  } else source.textContent = '来源：岗位知识推导 · 非公司真题';
  el('interview-status').textContent = `第 ${position + 1} / ${deck.length} 题 · 本轮不重复`;
  next.disabled = false;
  next.textContent = position === deck.length - 1 ? '完成本轮 ✓' : '下一题 →';
  el('question-prompt').focus();
}
el('start-interview').addEventListener('click', () => {
  deck = shuffle(
    topics
      .filter(
        (t) =>
          (!topicFilter.value || t.id === topicFilter.value) &&
          (!roleFilter.value || t.roles.includes(roleFilter.value)) &&
          (!companyFilter.value || t.companyTypes.includes(companyFilter.value)),
      )
      .flatMap((t) =>
        t.questions
          .filter((q) => !levelFilter.value || q.level === levelFilter.value)
          .map((q) => ({ ...q, topicId: t.id, title: t.title })),
      ),
  );
  position = 0;
  display();
});
next.addEventListener('click', () => {
  if (position < deck.length - 1) {
    position++;
    display();
  } else {
    next.disabled = true;
    el('interview-status').textContent =
      `本轮 ${deck.length} 题已完成。可调整范围重新抽题，草稿已保留。`;
  }
});
draft.addEventListener('input', () => {
  if (current) {
    const progress = loadProgress();
    progress.drafts[current.id] = draft.value;
    saveProgress(progress);
  }
});
for (const field of [topicFilter, levelFilter, roleFilter, companyFilter])
  field.addEventListener('change', () => {
    deck = [];
    current = undefined;
    card.hidden = true;
    el('interview-intro').hidden = false;
    el('interview-status').textContent = '筛选已更改，请开始新一轮。已有草稿保留。';
  });
