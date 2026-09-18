import { readFileSync, writeFileSync, unlinkSync, existsSync } from 'node:fs';
import { execFileSync } from 'node:child_process';
import matter from 'gray-matter';
import assert from 'node:assert/strict';
const source = matter(readFileSync('content/topics/concurrency/cpp-memory-model.md', 'utf8'));
const file = 'content/topics/concurrency/mdx-pipeline-probe.mdx';
if (existsSync(file)) throw new Error(`Refusing to overwrite an existing fixture: ${file}`);
source.data.id = 'mdx-pipeline-probe';
source.data.questions = source.data.questions.map((q, i) => ({ ...q, id: `mdx-probe-q${i}` }));
// Temporary fixture is removed even when compilation fails. Never published.
try {
  writeFileSync(
    file,
    `---\n${JSON.stringify(source.data)}\n---\n\nexport const message = 'MDX pipeline verified';\n\n<aside>{message}</aside>\n\n${source.content}`,
  );
  execFileSync(
    process.execPath,
    ['node_modules/astro/bin/astro.mjs', 'build', '--outDir', '.artifacts/mdx-dist'],
    { stdio: 'pipe' },
  );
  const html = readFileSync('.artifacts/mdx-dist/topics/mdx-pipeline-probe/index.html', 'utf8');
  assert.match(html, /MDX pipeline verified/);
  assert.match(html, /Spsc/);
  assert.match(html, /topics\/shared-mutex\//);
  console.log('MDX smoke passed: expression, tested C++ include and internal link rendered.');
} finally {
  unlinkSync(file);
}
