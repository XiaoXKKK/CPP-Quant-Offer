import { readdirSync, readFileSync } from 'node:fs';
import path from 'node:path';
import matter from 'gray-matter';
import { unified } from 'unified';
import remarkParse from 'remark-parse';
import remarkMdx from 'remark-mdx';
import { visit } from 'unist-util-visit';
import Slugger from 'github-slugger';
import { topicSchema } from '../src/data/schema.mjs';
import { sections } from '../src/data/taxonomy.mjs';

export function filesUnder(directory) {
  return readdirSync(directory, { withFileTypes: true }).flatMap((entry) =>
    entry.isDirectory()
      ? filesUnder(path.join(directory, entry.name))
      : [path.join(directory, entry.name)],
  );
}
export function parseMarkdown(file, text) {
  const processor = unified().use(remarkParse);
  if (file.endsWith('.mdx')) processor.use(remarkMdx);
  return processor.parse(text);
}
export function readTopics(root = 'content/topics') {
  return filesUnder(root)
    .filter((file) => /\.mdx?$/.test(file))
    .map((file) => {
      const raw = readFileSync(file, 'utf8');
      const { data, content } = matter(raw);
      return {
        file,
        raw,
        body: content,
        data: topicSchema.parse(data),
        tree: parseMarkdown(file, content),
      };
    });
}
export function nodeText(node) {
  return node.value || (node.children || []).map(nodeText).join('');
}
export function headings(tree) {
  const slugger = new Slugger();
  const result = [];
  visit(tree, 'heading', (node) => {
    const text = nodeText(node);
    result.push({ depth: node.depth, text, id: slugger.slug(text) });
  });
  return result;
}
export function validateTopics(topics) {
  const errors = [];
  const ids = new Set();
  const questionIds = new Set();
  const byFile = new Map(topics.map((t) => [path.resolve(t.file), t]));
  for (const topic of topics) {
    const { data, tree, file } = topic;
    const fail = (message) => errors.push(`${file}: ${message}`);
    if (ids.has(data.id)) fail(`Duplicate topic id ${data.id}`);
    ids.add(data.id);
    if (path.basename(file).replace(/\.mdx?$/, '') !== data.id)
      fail('Filename must match stable id');
    if (path.basename(path.dirname(file)) !== data.category) fail('Directory must match category');
    const actual = headings(tree)
      .filter((h) => h.depth === 2)
      .map((h) => h.text);
    if (JSON.stringify(actual) !== JSON.stringify(sections))
      fail('Required H2 sections must appear exactly once, in canonical order');
    let section = '';
    let length = 0;
    for (const node of tree.children) {
      if (node.type === 'heading' && node.depth === 2) {
        if (section && length < 20) fail(`Empty or trivial section: ${section}`);
        section = nodeText(node);
        length = 0;
      } else
        length +=
          nodeText(node).length +
          (node.type === 'code' && node.meta?.startsWith('include=') ? 100 : 0);
    }
    if (section && length < 20) fail(`Empty or trivial section: ${section}`);
    let demoCount = 0;
    const definitions = new Map();
    visit(tree, 'definition', (node) => definitions.set(node.identifier, node.url));
    visit(tree, (node) => {
      if (
        node.type === 'html' &&
        /<(script|iframe|style|object)\b|\son\w+\s*=|javascript:/i.test(node.value)
      )
        fail('Unsafe HTML in content');
      if (node.type === 'code') {
        if (!node.lang) fail('Every fenced block needs a language');
        if (['cpp', 'c++', 'cxx'].includes(node.lang)) {
          if (node.lang !== 'cpp') fail('Use cpp as the canonical C++ fence language');
          const opening = topic.body.split('\n')[node.position.start.line - 1];
          if (!/^```cpp(?: |$)/.test(opening))
            fail('Use unindented triple-backtick cpp fences so the compiler test can extract them');
          if (
            node.lang === 'cpp' &&
            node.meta === `include=${data.demo.file}` &&
            !node.value.trim()
          )
            demoCount++;
          else if (node.meta !== 'compile-only' && node.meta !== 'runnable')
            fail('C++ blocks must reference the canonical demo or declare compile-only/runnable');
        }
      }
      if (!['link', 'image', 'linkReference', 'imageReference'].includes(node.type)) return;
      const url = node.url || definitions.get(node.identifier);
      if (!url) {
        fail('Unresolved reference link');
        return;
      }
      if (/^(https?:|mailto:)/.test(url)) return;
      if (/^[a-z]+:/i.test(url)) {
        fail(`Disallowed URL: ${url}`);
        return;
      }
      const [target, hash] = url.split('#');
      const targetPath = path.resolve(
        path.dirname(file),
        decodeURIComponent(target || path.basename(file)),
      );
      const other = byFile.get(targetPath);
      if (target && !other) {
        fail(`Unknown internal topic link: ${url}`);
        return;
      }
      if (other?.data.status === 'draft' && data.status === 'published')
        fail('Published topic links to a draft');
      const fragmentIds = [
        ...headings((other || topic).tree).map((h) => h.id),
        ...(other || topic).data.questions.map((q) => q.id),
        'code-lab',
        'question-bank',
        'references',
      ];
      if (hash && !fragmentIds.includes(decodeURIComponent(hash))) fail(`Broken anchor: ${url}`);
    });
    if (demoCount !== 1) fail('Exactly one canonical demo include is required');
    try {
      if (!readFileSync(data.demo.file, 'utf8').includes('int main('))
        fail('Demo must have main()');
    } catch {
      fail(`Missing demo ${data.demo.file}`);
    }
    for (const q of data.questions) {
      if (questionIds.has(q.id)) fail(`Global question id collision: ${q.id}`);
      questionIds.add(q.id);
    }
    for (const related of data.related) {
      if (related === data.id) fail('Self-related topic');
      if (
        !topics.some(
          (t) =>
            t.data.id === related && (data.status === 'draft' || t.data.status === 'published'),
        )
      )
        fail(`Missing or unpublished related topic: ${related}`);
    }
  }
  return errors;
}
