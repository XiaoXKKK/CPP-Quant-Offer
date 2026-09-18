import { z } from 'zod';
import { categories, roles, companyTypes } from './taxonomy.mjs';

const id = z.string().regex(/^[a-z0-9]+(?:-[a-z0-9]+)*$/);
const date = z.iso.date();
const url = z.url().refine((value) => value.startsWith('https://'), 'Sources must use HTTPS');
const source = z.discriminatedUnion('kind', [
  z.object({ kind: z.literal('derived'), rationale: z.string().min(12) }).strict(),
  z
    .object({
      kind: z.literal('public-interview'),
      url,
      title: z.string().min(3),
      accessed: date,
      published: date.nullable(),
      interviewDate: date.nullable(),
      note: z.string().min(12),
    })
    .strict(),
]);
export const questionSchema = z
  .object({
    id,
    level: z.enum(['L1', 'L2', 'L3']),
    prompt: z.string().min(8),
    answer: z.string().min(30),
    rubric: z.array(z.string().min(2)).min(2),
    source,
    companies: z.array(z.string().min(2)),
  })
  .strict()
  .superRefine((q, ctx) => {
    if (q.source.kind === 'derived' && q.companies.length)
      ctx.addIssue({
        code: 'custom',
        message: 'Derived questions cannot claim a company attribution',
      });
  });
export const topicSchema = z
  .object({
    schemaVersion: z.literal(1),
    id,
    title: z.string().min(4),
    description: z.string().min(20),
    category: z.enum(categories.map((c) => c.id)),
    areas: z.array(z.string()).min(1),
    tags: z.array(id).min(2),
    difficulty: z.enum(['L1', 'L2', 'L3']),
    roles: z.array(z.enum(roles)).min(1),
    companyTypes: z.array(z.enum(companyTypes)).min(1),
    status: z.enum(['draft', 'published']),
    updated: date,
    reviewed: date,
    standard: z.enum(['C++17', 'C++20', 'C++23']),
    estimatedMinutes: z.number().int().min(10).max(180),
    prerequisites: z.array(z.string().min(2)),
    related: z.array(id).min(1),
    demo: z
      .object({
        file: z.string().regex(/^examples\/[a-z0-9-]+\.cpp$/),
        platform: z.enum(['portable', 'linux']),
        exercise: z.string().min(20),
      })
      .strict(),
    references: z
      .array(
        z
          .object({
            title: z.string().min(3),
            url,
            accessed: date,
            kind: z.enum(['standard', 'manual', 'protocol', 'implementation']),
          })
          .strict(),
      )
      .min(2),
    questions: z.array(questionSchema).min(15).max(30),
  })
  .strict()
  .superRefine((t, ctx) => {
    const category = categories.find((c) => c.id === t.category);
    if (t.areas.some((a) => !category.areas.includes(a)))
      ctx.addIssue({ code: 'custom', message: 'Area must belong to the selected category' });
    if (new Set(t.questions.map((q) => q.id)).size !== t.questions.length)
      ctx.addIssue({ code: 'custom', message: 'Duplicate question IDs' });
    for (const level of ['L1', 'L2', 'L3'])
      if (t.questions.filter((q) => q.level === level).length < 5)
        ctx.addIssue({ code: 'custom', message: `At least five ${level} questions required` });
    if (t.status === 'published' && t.reviewed < t.updated)
      ctx.addIssue({
        code: 'custom',
        message: 'Published content must be reviewed on or after its update date',
      });
    const today = new Date().toISOString().slice(0, 10);
    if (t.updated > today || t.reviewed > today)
      ctx.addIssue({ code: 'custom', message: 'Future publication/review date' });
    for (const ref of t.references)
      if (ref.accessed > today)
        ctx.addIssue({ code: 'custom', message: 'Future reference access date' });
    for (const q of t.questions)
      if (q.source.kind === 'public-interview') {
        if (
          q.source.accessed > today ||
          (q.source.published && q.source.published > q.source.accessed) ||
          (q.source.interviewDate && q.source.interviewDate > q.source.accessed)
        )
          ctx.addIssue({ code: 'custom', message: 'Inconsistent interview source dates' });
      }
  });
