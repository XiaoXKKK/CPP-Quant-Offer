import { loadProgress, saveProgress, notify } from './progress';
import { parseProgress } from '../lib/study.mjs';

document.querySelector('#import-trigger')?.addEventListener('click', () => {
  document.querySelector<HTMLInputElement>('#import-progress')?.click();
});
document.addEventListener('keydown', (event) => {
  if (event.key === 'Escape') {
    document.querySelector('#sidebar')?.classList.remove('is-open');
    document.querySelector('.menu-button')?.setAttribute('aria-expanded', 'false');
  }
});

document.querySelector('.menu-button')?.addEventListener('click', (event) => {
  const button = event.currentTarget as HTMLButtonElement;
  const open = button.getAttribute('aria-expanded') !== 'true';
  button.setAttribute('aria-expanded', String(open));
  document.querySelector('#sidebar')?.classList.toggle('is-open', open);
});
document.querySelector('#export-progress')?.addEventListener('click', () => {
  const url = URL.createObjectURL(
    new Blob([JSON.stringify(loadProgress(), null, 2)], { type: 'application/json' }),
  );
  const link = document.createElement('a');
  link.href = url;
  link.download = 'cpp-quant-offer-study.json';
  link.click();
  setTimeout(() => URL.revokeObjectURL(url), 1000);
});
document.querySelector('#import-progress')?.addEventListener('change', async (event) => {
  const input = event.target as HTMLInputElement;
  const file = input.files?.[0];
  if (!file) return;
  try {
    if (file.size > 5_000_000) throw new Error('备份不能超过 5 MB');
    const incoming = parseProgress(await file.text());
    const existing = loadProgress();
    saveProgress({
      version: 1,
      read: [...new Set([...existing.read, ...incoming.read])],
      saved: [...new Set([...existing.saved, ...incoming.saved])],
      drafts: { ...incoming.drafts, ...existing.drafts },
      code: { ...incoming.code, ...existing.code },
    });
    notify('备份已合并；同名作答和代码保留当前浏览器版本。');
  } catch (error) {
    notify(`导入失败：${error instanceof Error ? error.message : '格式错误'}`);
  }
  input.value = '';
});

function refreshButtons() {
  const progress = loadProgress();
  document.querySelectorAll<HTMLButtonElement>('[data-progress]').forEach((button) => {
    const type = button.dataset.progress as 'read' | 'saved';
    const selected = progress[type].includes(button.dataset.topic!);
    button.setAttribute('aria-pressed', String(selected));
    button.textContent =
      type === 'read' ? (selected ? '✓ 已读' : '标记已读') : selected ? '★ 已收藏' : '☆ 收藏';
  });
}
document.querySelectorAll<HTMLButtonElement>('[data-progress]').forEach((button) => {
  button.addEventListener('click', () => {
    const state = loadProgress();
    const type = button.dataset.progress as 'read' | 'saved';
    const id = button.dataset.topic!;
    state[type] = state[type].includes(id)
      ? state[type].filter((v) => v !== id)
      : [...state[type], id];
    saveProgress(state);
  });
});
document.querySelectorAll<HTMLTextAreaElement>('[data-draft]').forEach((field) => {
  field.value = loadProgress().drafts[field.dataset.draft!] || '';
  field.addEventListener('input', () => {
    const state = loadProgress();
    state.drafts[field.dataset.draft!] = field.value;
    saveProgress(state);
  });
});
document.addEventListener('progress-changed', refreshButtons);
refreshButtons();
