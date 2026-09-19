import { loadProgress, saveProgress, notify } from './progress';
import { formatCompilerResult } from '../lib/compiler.mjs';
import type { editor as MonacoEditor } from 'monaco-editor';
const lab = document.querySelector<HTMLElement>('.code-lab');
if (lab) {
  const id = lab.dataset.codeId!;
  const editor = lab.querySelector<HTMLTextAreaElement>('#code-editor')!;
  const output = lab.querySelector<HTMLElement>('#code-output')!;
  const run = lab.querySelector<HTMLButtonElement>('#run-code')!;
  const dialog = lab.querySelector<HTMLDialogElement>('.lab-dialog')!;
  const host = lab.querySelector<HTMLElement>('#monaco-editor')!;
  const saveStatus = lab.querySelector<HTMLElement>('#code-save-status')!;
  let monaco: MonacoEditor.IStandaloneCodeEditor | undefined;
  let loading: Promise<void> | undefined;
  let previousHash = location.hash === '#code-lab' ? '' : location.hash;
  let returnFocus: HTMLElement | null = null;
  const value = () => monaco?.getValue() ?? editor.value;
  editor.value = loadProgress().code[id] ?? editor.value;
  function save() {
    editor.value = value();
    if (editor.value.length > 100000) {
      saveStatus.textContent = '超过 100,000 字符，当前修改未保存';
      return;
    }
    const progress = loadProgress();
    progress.code[id] = editor.value;
    const saved = saveProgress(progress);
    saveStatus.textContent = saved ? '已保存到此浏览器' : '暂存于当前页面';
  }
  editor.addEventListener('input', save);
  async function initEditor() {
    try {
      const { createEditor } = await import('./monaco');
      host.hidden = false;
      monaco = createEditor(host, editor.value, () => run.click());
      editor.hidden = true;
      monaco.onDidChangeModelContent(save);
    } catch {
      host.hidden = true;
      editor.hidden = false;
      saveStatus.textContent = '高亮加载失败，可继续编辑和运行';
      loading = undefined;
    }
  }
  function openLab(trigger: HTMLElement | null = null) {
    if (dialog.open) return;
    returnFocus = trigger;
    if (location.hash !== '#code-lab') {
      previousHash = location.hash;
      history.pushState(null, '', '#code-lab');
    }
    document.documentElement.classList.add('lab-open');
    document.body.classList.add('lab-open');
    window.scrollTo({ top: window.scrollY, behavior: 'instant' });
    dialog.showModal();
    loading ??= initEditor();
    void loading.then(() => monaco?.layout());
  }
  document.querySelectorAll<HTMLAnchorElement>('a[href="#code-lab"]').forEach((link) => {
    link.addEventListener('click', (event) => {
      event.preventDefault();
      openLab(link);
    });
  });
  lab.querySelector('#close-lab')!.addEventListener('click', () => dialog.close());
  dialog.addEventListener('close', () => {
    document.body.classList.remove('lab-open');
    document.documentElement.classList.remove('lab-open');
    if (location.hash === '#code-lab')
      history.replaceState(null, '', `${location.pathname}${location.search}${previousHash}`);
    returnFocus?.focus({ preventScroll: true });
  });
  window.addEventListener('hashchange', () => {
    if (location.hash === '#code-lab') openLab();
    else if (dialog.open) dialog.close();
  });
  if (location.hash === '#code-lab') openLab();
  dialog.addEventListener('keydown', (event) => {
    if ((event.ctrlKey || event.metaKey) && event.key === 'Enter') {
      event.preventDefault();
      run.click();
    }
  });
  const divider = lab.querySelector<HTMLElement>('.lab-divider')!;
  const workspace = lab.querySelector<HTMLElement>('.lab-workspace')!;
  function resize(percent: number) {
    const width = Math.max(25, Math.min(65, percent));
    workspace.style.setProperty('--lab-left', `${width}%`);
    divider.setAttribute('aria-valuenow', String(Math.round(width)));
  }
  divider.addEventListener('pointerdown', (event) => {
    divider.setPointerCapture(event.pointerId);
    event.preventDefault();
  });
  divider.addEventListener('pointermove', (event) => {
    if (!divider.hasPointerCapture(event.pointerId)) return;
    const bounds = workspace.getBoundingClientRect();
    resize(((event.clientX - bounds.left) / bounds.width) * 100);
  });
  divider.addEventListener('pointerup', (event) => {
    if (divider.hasPointerCapture(event.pointerId)) divider.releasePointerCapture(event.pointerId);
  });
  divider.addEventListener('keydown', (event) => {
    if (event.key !== 'ArrowLeft' && event.key !== 'ArrowRight') return;
    event.preventDefault();
    resize(Number(divider.getAttribute('aria-valuenow')) + (event.key === 'ArrowLeft' ? -2 : 2));
  });
  lab.querySelector('#copy-code')?.addEventListener('click', async () => {
    try {
      await navigator.clipboard.writeText(value());
      saveStatus.textContent = '代码已复制';
      notify('代码已复制。');
    } catch {
      if (monaco) {
        monaco.focus();
        monaco.setSelection(monaco.getModel()!.getFullModelRange());
      } else {
        editor.focus();
        editor.select();
      }
      notify('无法访问剪贴板，已选中代码，请手动复制。');
      saveStatus.textContent = '已选中代码，请手动复制';
    }
  });
  run.addEventListener('click', async () => {
    const source = value();
    if (!source.trim()) {
      output.textContent = '请输入 C++ 代码后再运行。';
      return;
    }
    if (source.length > 100000) {
      output.textContent = '代码超过 100,000 字符，请缩短后运行。';
      return;
    }
    run.disabled = true;
    output.textContent = '编译和运行中…（最多等待 30 秒）';
    try {
      const response = await fetch('https://godbolt.org/api/compiler/g132/compile', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json', Accept: 'application/json' },
        signal: AbortSignal.timeout(30000),
        body: JSON.stringify({
          source,
          lang: 'c++',
          options: {
            userArguments: '-std=c++20 -O2 -Wall -Wextra -pthread',
            compilerOptions: { executorRequest: true },
            filters: { execute: true },
            executeParameters: { args: [], stdin: '' },
          },
        }),
      });
      if (!response.ok) throw new Error(`远程服务 HTTP ${response.status}`);
      const result = await response.json();
      output.textContent = formatCompilerResult(result);
    } catch (error) {
      output.textContent = `运行未完成：${error instanceof Error ? error.message : '网络错误'}\n代码已保留。请复制到 Compiler Explorer 或 Linux / WSL 本地编译；不要把服务失败当成代码错误。`;
    } finally {
      run.disabled = false;
    }
  });
}
