import { getCollection } from 'astro:content';
export async function publishedTopics() {
  return (await getCollection('topics', ({ data }) => data.status === 'published')).sort((a, b) =>
    a.data.id.localeCompare(b.data.id),
  );
}
export function safeJson(value: unknown) {
  return JSON.stringify(value).replace(/</g, '\\u003c');
}
