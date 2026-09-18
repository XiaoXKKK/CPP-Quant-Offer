export function normalize(value) {
  return value.normalize('NFKC').toLocaleLowerCase().replace(/\s+/g, ' ').trim();
}

/** @param {{read: string[], saved: string[]}} state */
export function matchesTopic(topic, filters, state = { read: [], saved: [] }) {
  const corpus = normalize(
    [
      topic.title,
      topic.description,
      topic.body,
      ...topic.tags,
      ...topic.roles,
      ...topic.companyTypes,
      ...topic.questions.flatMap((q) => [q.prompt, q.answer, ...q.companies]),
    ].join(' '),
  );
  return (
    normalize(filters.query || '')
      .split(' ')
      .filter(Boolean)
      .every((word) => corpus.includes(word)) &&
    (!filters.category || topic.category === filters.category) &&
    (!filters.tag || topic.tags.includes(filters.tag)) &&
    (!filters.level || topic.difficulty === filters.level) &&
    (!filters.role || topic.roles.includes(filters.role)) &&
    (!filters.companyType || topic.companyTypes.includes(filters.companyType)) &&
    (!filters.company || topic.questions.some((q) => q.companies.includes(filters.company))) &&
    (!filters.status ||
      (filters.status === 'saved'
        ? state.saved.includes(topic.id)
        : filters.status === 'read'
          ? state.read.includes(topic.id)
          : !state.read.includes(topic.id)))
  );
}

export function shuffle(items, random = Math.random) {
  const result = [...items];
  for (let i = result.length - 1; i > 0; i--) {
    const j = Math.floor(random() * (i + 1));
    [result[i], result[j]] = [result[j], result[i]];
  }
  return result;
}

export const emptyProgress = () => ({ version: 1, read: [], saved: [], drafts: {}, code: {} });
export function parseProgress(raw) {
  const value = typeof raw === 'string' ? JSON.parse(raw) : raw;
  const validKey = (key) => /^[a-z0-9]+(?:-[a-z0-9]+)*$/.test(key);
  if (!value || value.version !== 1) throw new Error('不支持的备份版本');
  for (const key of ['read', 'saved']) {
    if (
      !Array.isArray(value[key]) ||
      value[key].length > 10000 ||
      !value[key].every((v) => typeof v === 'string' && validKey(v))
    )
      throw new Error('学习记录格式错误');
  }
  for (const key of ['drafts', 'code']) {
    if (!value[key] || typeof value[key] !== 'object' || Array.isArray(value[key]))
      throw new Error('草稿格式错误');
    if (
      Object.entries(value[key]).some(
        ([k, v]) => !validKey(k) || typeof v !== 'string' || v.length > 100000,
      )
    )
      throw new Error('草稿字段无效或过长');
  }
  return {
    version: 1,
    read: [...new Set(value.read)],
    saved: [...new Set(value.saved)],
    drafts: { ...value.drafts },
    code: { ...value.code },
  };
}

export function searchExcerpt(topic, query) {
  const text =
    `${topic.description} ${topic.body} ${topic.questions.map((q) => `${q.prompt} ${q.answer}`).join(' ')}`
      .replace(/[#`*\n]/g, ' ')
      .replace(/\s+/g, ' ');
  const term = normalize(query).split(' ').find(Boolean);
  const index = term ? normalize(text).indexOf(term) : 0;
  const start = Math.max(0, index - 35);
  return `${start ? '…' : ''}${text.slice(start, start + 150)}${text.length > start + 150 ? '…' : ''}`;
}
