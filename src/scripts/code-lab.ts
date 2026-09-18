import { loadProgress, saveProgress, notify } from './progress';
import { formatCompilerResult } from '../lib/compiler.mjs';
const lab = document.querySelector<HTMLElement>('.code-lab');
if (lab) {
  const id = lab.dataset.codeId!;
  const editor = lab.querySelector<HTMLTextAreaElement>('#code-editor')!;
  const output = lab.querySelector<HTMLElement>('#code-output')!;
  const run = lab.querySelector<HTMLButtonElement>('#run-code')!;
  editor.value = loadProgress().code[id] ?? editor.value;
  editor.addEventListener('input', () => {
    const progress = loadProgress();
    progress.code[id] = editor.value;
    saveProgress(progress);
  });
  lab.querySelector('#copy-code')?.addEventListener('click', async () => {
    try {
      await navigator.clipboard.writeText(editor.value);
      notify('代码已复制。');
    } catch {
      editor.focus();
      editor.select();
      notify('无法访问剪贴板，已选中代码，请手动复制。');
    }
  });
  run.addEventListener('click', async () => {
    if (!editor.value.trim()) {
      output.textContent = '请输入 C++ 代码后再运行。';
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
          source: editor.value,
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
