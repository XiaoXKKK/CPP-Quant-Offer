export function formatCompilerResult(result) {
  const lines = (value) => (value || []).map((line) => line.text || '').join('\n');
  // Executor-only requests return execution at the top level, with buildResult nested.
  // Compile-and-execute requests may instead return a nested execResult.
  const build = result.buildResult || result;
  const execution = result.execResult || (result.didExecute ? result : null);
  return [
    `编译退出码：${build.code ?? '未知'}`,
    build === execution ? '' : lines(build.stdout),
    build === execution ? '' : lines(build.stderr),
    execution
      ? `运行退出码：${execution.code ?? '未知'}\n${lines(execution.stdout)}\n${lines(execution.stderr)}`
      : '未运行；请先检查编译诊断。若编译成功仍没有运行结果，请在 Compiler Explorer 页面或本地运行。',
    build.timedOut || execution?.timedOut ? '远程编译或执行超时。' : '',
    build.truncated || execution?.truncated ? '远程输出已截断。' : '',
  ]
    .filter(Boolean)
    .join('\n')
    .slice(0, 30000);
}
