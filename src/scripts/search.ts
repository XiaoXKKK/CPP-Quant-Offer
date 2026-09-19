import { matchesTopic, searchExcerpt } from '../lib/study.mjs';
import { loadProgress } from './progress';
const form = document.querySelector<HTMLFormElement>('#search-form')!;
const topics = JSON.parse(document.querySelector('#search-data')!.textContent!);
const query = document.querySelector<HTMLInputElement>('#search')!;
const fields = ['category', 'tag', 'level', 'status'];
function restore() {
  const params = new URLSearchParams(location.search);
  query.value = params.get('q') || '';
  for (const name of fields)
    (form.elements.namedItem(name) as HTMLSelectElement).value = params.get(name) || '';
}
function update(writeUrl = true) {
  const filters: Record<string, string> = { query: query.value };
  const params = new URLSearchParams();
  if (query.value) params.set('q', query.value);
  for (const name of fields) {
    const value = (form.elements.namedItem(name) as HTMLSelectElement).value;
    filters[name] = value;
    if (value) params.set(name, value);
  }
  let count = 0;
  for (const topic of topics) {
    const row = document.querySelector<HTMLElement>(`[data-topic-row="${topic.id}"]`)!;
    row.hidden = !matchesTopic(topic, filters, loadProgress());
    if (!row.hidden) count++;
    const excerpt = row.querySelector<HTMLElement>('.search-excerpt')!;
    excerpt.hidden = !query.value.trim();
    excerpt.textContent = query.value.trim() ? searchExcerpt(topic, query.value) : '';
  }
  document.querySelector('#result-count')!.textContent =
    `${count} 篇匹配专题 / ${topics.length} 篇已发布`;
  (document.querySelector('#empty-state') as HTMLElement).hidden = count > 0;
  if (writeUrl)
    history.replaceState(null, '', `${location.pathname}${params.size ? `?${params}` : ''}`);
}
form.addEventListener('input', () => update());
form.addEventListener('change', () => update());
form.addEventListener('submit', (event) => {
  event.preventDefault();
  update();
});
form.addEventListener('reset', () => setTimeout(update, 0));
document.addEventListener('progress-changed', () => update(false));
window.addEventListener('popstate', () => {
  restore();
  update(false);
});
document.addEventListener('keydown', (event) => {
  const target = event.target as HTMLElement;
  if (
    event.key === '/' &&
    !['INPUT', 'TEXTAREA', 'SELECT'].includes(target.tagName) &&
    !target.isContentEditable
  ) {
    event.preventDefault();
    query.focus();
  }
});
restore();
update(false);
