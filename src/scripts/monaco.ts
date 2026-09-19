import * as monaco from 'monaco-editor/editor/editor.api';
import 'monaco-editor/languages/definitions/cpp/register';
import 'monaco-editor/editor/contrib/find/browser/findController';
import 'monaco-editor/editor/contrib/bracketMatching/browser/bracketMatching';
import 'monaco-editor/editor/contrib/wordOperations/browser/wordOperations';
import 'monaco-editor/editor/contrib/linesOperations/browser/linesOperations';
import 'monaco-editor/editor/contrib/comment/browser/comment';
import 'monaco-editor/editor/contrib/folding/browser/folding';
import 'monaco-editor/editor/contrib/suggest/browser/suggestController';
import 'monaco-editor/editor/contrib/toggleTabFocusMode/browser/toggleTabFocusMode';
import 'monaco-editor/editor/contrib/contextmenu/browser/contextmenu';
import EditorWorker from 'monaco-editor/editor/editor.worker?worker';

self.MonacoEnvironment = { getWorker: () => new EditorWorker() };

export function createEditor(container: HTMLElement, value: string, onRun: () => void) {
  const editor = monaco.editor.create(container, {
    value,
    language: 'cpp',
    theme: 'vs-dark',
    automaticLayout: true,
    minimap: { enabled: false },
    fontFamily: "'Cascadia Code', Consolas, monospace",
    fontSize: 14,
    lineHeight: 23,
    tabSize: 4,
    insertSpaces: true,
    scrollBeyondLastLine: false,
    padding: { top: 16, bottom: 16 },
    bracketPairColorization: { enabled: true },
    wordWrap: 'off',
    ariaLabel: 'C++ 代码编辑器',
    fixedOverflowWidgets: true,
  });
  editor.addAction({
    id: 'codelab.run',
    label: '运行代码',
    keybindings: [monaco.KeyMod.CtrlCmd | monaco.KeyCode.Enter],
    run: onRun,
  });
  return editor;
}
