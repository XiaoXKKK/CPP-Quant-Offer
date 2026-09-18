import { defineCollection } from 'astro:content';
import { glob } from 'astro/loaders';
import { topicSchema } from './data/schema.mjs';
const topics = defineCollection({
  loader: glob({ pattern: '**/*.{md,mdx}', base: './content/topics' }),
  schema: topicSchema,
});
export const collections = { topics };
