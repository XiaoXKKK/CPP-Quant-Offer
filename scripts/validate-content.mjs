import {
  readTopics,
  validateTopics,
  filesUnder,
  parseMarkdown,
  headings,
  nodeText,
} from './content.mjs';
import { readFileSync, existsSync } from 'node:fs';
import path from 'node:path';
import { visit } from 'unist-util-visit';
import { createHash } from 'node:crypto';

try {
  const topics = readTopics();
  const errors = validateTopics(topics);
  const docs = [
    'README.md',
    'CONTRIBUTING.md',
    ...filesUnder('docs').filter((f) => /\.md$/.test(f)),
  ];
  for (const file of docs) {
    const tree = parseMarkdown(file, readFileSync(file, 'utf8'));
    const definitions = new Map();
    visit(tree, 'definition', (node) => definitions.set(node.identifier, node.url));
    visit(tree, (node) => {
      let url = ['link', 'image'].includes(node.type)
        ? node.url
        : ['linkReference', 'imageReference'].includes(node.type)
          ? definitions.get(node.identifier)
          : undefined;
      if (url === undefined) return;
      if (/^(https?:|mailto:)/.test(url)) return;
      if (/^[a-z]+:/i.test(url)) {
        errors.push(`${file}: unsafe URL ${url}`);
        return;
      }
      const [target, hash] = url.split('#');
      const dest = path.resolve(
        path.dirname(file),
        decodeURIComponent(target || path.basename(file)),
      );
      if (!existsSync(dest)) {
        errors.push(`${file}: missing link ${url}`);
        return;
      }
      if (hash && /\.md$/.test(dest)) {
        const ids = headings(parseMarkdown(dest, readFileSync(dest, 'utf8'))).map((h) => h.id);
        if (!ids.includes(decodeURIComponent(hash))) errors.push(`${file}: missing anchor ${url}`);
      }
    });
    visit(tree, 'code', (node) => {
      if (!node.lang) errors.push(`${file}: code block language missing`);
    });
    if (!nodeText(tree)) errors.push(`${file}: empty document`);
  }
  // Review records are honest self-review evidence, bound to exact content and demo bytes.
  const manifest = JSON.parse(readFileSync('reviews/content-review.json', 'utf8'));
  for (const topic of topics.filter((t) => t.data.status === 'published')) {
    for (const file of [topic.file.replaceAll('\\', '/'), topic.data.demo.file]) {
      const hash = createHash('sha256').update(readFileSync(file)).digest('hex');
      if (manifest.files[file] !== hash)
        errors.push(`${file}: review hash is stale; review before updating the record`);
    }
  }
  if (manifest.kind !== 'self-review' || manifest.verdict !== 'pass')
    errors.push('Review record must honestly identify its kind and pass verdict');
  if (errors.length) throw new Error(errors.join('\n'));
  console.log(
    `Content OK: ${topics.length} topics, ${topics.reduce((n, t) => n + t.data.questions.length, 0)} questions; schema, sections, links, code and review hashes checked.`,
  );
} catch (error) {
  console.error(error instanceof Error ? error.message : error);
  process.exitCode = 1;
}
