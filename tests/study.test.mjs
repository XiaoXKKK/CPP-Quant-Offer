import test from 'node:test';
import assert from 'node:assert/strict';
import { matchesTopic, parseProgress, emptyProgress, shuffle } from '../src/lib/study.mjs';
import { readTopics, validateTopics } from '../scripts/content.mjs';
import { topicSchema } from '../src/data/schema.mjs';
import { formatCompilerResult } from '../src/lib/compiler.mjs';
test('Compiler Explorer distinguishes build status from direct executor exit status', () => {
  const output = formatCompilerResult({
    code: 7,
    didExecute: true,
    buildResult: { code: 0, stderr: [] },
    stdout: [{ text: 'hello' }],
  });
  assert.match(output, /编译退出码：0/);
  assert.match(output, /运行退出码：7/);
  assert.match(output, /hello/);
  assert.match(
    formatCompilerResult({ code: 1, stderr: [{ text: 'syntax error' }] }),
    /syntax error/,
  );
  assert.match(
    formatCompilerResult({ code: 0, execResult: { code: 0, stdout: [{ text: 'nested' }] } }),
    /nested/,
  );
});
const topics = readTopics();
test('all exemplar structure, links and runnable demo references are valid', () =>
  assert.deepEqual(validateTopics(topics), []));
test('full-text search includes body and answers, with Chinese and AND tokens', () => {
  const topic = {
    ...topics.find((t) => t.data.id === 'cpp-memory-model').data,
    body: '发布到槽位',
  };
  assert.equal(matchesTopic(topic, { query: '发布 槽位' }), true);
  assert.equal(matchesTopic(topic, { query: 'seq_cst' }), true);
  assert.equal(matchesTopic(topic, { query: '发布 不存在的词' }), false);
  assert.equal(matchesTopic(topic, { role: 'Quant Developer', category: 'network' }), false);
  assert.equal(matchesTopic(topic, { status: 'saved' }, { read: [], saved: [topic.id] }), true);
});
test('question provenance blocks invented company attribution and incomplete public sources', () => {
  const data = structuredClone(topics[0].data);
  data.questions[0].companies = ['Example firm'];
  assert.equal(topicSchema.safeParse(data).success, false);
  data.questions[0].companies = [];
  data.questions[0].source = { kind: 'public-interview', url: 'https://example.com' };
  assert.equal(topicSchema.safeParse(data).success, false);
});
test('schema rejects duplicate questions, missing layers, wrong area and unsafe demo paths', () => {
  for (const mutate of [
    (t) => (t.questions[1].id = t.questions[0].id),
    (t) => t.questions.forEach((q) => (q.level = 'L1')),
    (t) => (t.areas = ['Imaginary']),
    (t) => (t.demo.file = '../private.cpp'),
  ]) {
    const data = structuredClone(topics[0].data);
    mutate(data);
    assert.equal(topicSchema.safeParse(data).success, false);
  }
});
test('content checks reject missing sections, broken anchors and missing related IDs', () => {
  const changed = structuredClone(topics);
  changed[0].data.related.push('does-not-exist');
  changed[0].tree.children.push({ type: 'link', url: '#missing-anchor', children: [] });
  changed[0].tree.children = changed[0].tree.children.filter(
    (n) => !(n.type === 'heading' && n.depth === 2),
  );
  const errors = validateTopics(changed).join('\n');
  assert.match(errors, /Required H2/);
  assert.match(errors, /Broken anchor/);
  assert.match(errors, /related/);
});
test('random interview uses a non-mutating permutation', () => {
  const input = Array.from({ length: 75 }, (_, i) => i);
  const result = shuffle(input, () => 0.2);
  assert.equal(new Set(result).size, 75);
  assert.deepEqual(
    [...result].sort((a, b) => a - b),
    input,
  );
  assert.notDeepEqual(result, input);
});
test('progress parsing rejects corrupted, unsafe or incompatible backups', () => {
  assert.deepEqual(parseProgress(JSON.stringify(emptyProgress())), emptyProgress());
  for (const value of [
    'not json',
    '{"version":2}',
    JSON.stringify({ ...emptyProgress(), saved: 'bad' }),
    '{"version":1,"read":[],"saved":[],"drafts":{"__proto__":"x"},"code":{}}',
  ])
    assert.throws(() => parseProgress(value));
});
