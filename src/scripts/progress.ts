import { emptyProgress, parseProgress } from '../lib/study.mjs';
export const key = 'cpp-quant-offer:study:v1';
export type Progress = {
  version: number;
  read: string[];
  saved: string[];
  drafts: Record<string, string>;
  code: Record<string, string>;
};
let memory: Progress | undefined;
let notificationTimer: ReturnType<typeof setTimeout> | undefined;
export function notify(message: string) {
  const node = document.querySelector<HTMLElement>('#global-status');
  if (node) {
    node.textContent = message;
    node.hidden = false;
    clearTimeout(notificationTimer);
    notificationTimer = setTimeout(() => {
      node.hidden = true;
    }, 8000);
  }
}
export function loadProgress(): Progress {
  if (memory) return memory;
  try {
    const raw = localStorage.getItem(key);
    memory = raw ? parseProgress(raw) : emptyProgress();
  } catch {
    memory = emptyProgress();
    notify('本地记录无法读取，当前改用临时记录。请导出备份后检查浏览器存储权限。');
  }
  return memory!;
}
export function saveProgress(progress: Progress) {
  memory = progress;
  let saved = true;
  try {
    localStorage.setItem(key, JSON.stringify(progress));
  } catch {
    saved = false;
    notify('浏览器未能保存记录；本页临时保留，请导出备份。');
  }
  document.dispatchEvent(new Event('progress-changed'));
  return saved;
}
window.addEventListener('storage', (event) => {
  if (event.key !== key) return;
  memory = undefined;
  document.dispatchEvent(new Event('progress-changed'));
});
