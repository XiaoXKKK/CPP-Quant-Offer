import { z } from 'zod';
import { mkdirSync, writeFileSync } from 'node:fs';
import { topicSchema } from '../src/data/schema.mjs';
mkdirSync('schemas', { recursive: true });
const schema = z.toJSONSchema(topicSchema, { target: 'draft-2020-12' });
schema.$id = 'https://xiaoxkkk.github.io/CPP-Quant-Offer/schemas/topic.schema.json';
schema.description =
  'Editor schema. Cross-field, references, dates and semantic checks also run via npm run validate.';
writeFileSync('schemas/topic.schema.json', `${JSON.stringify(schema, null, 2)}\n`);
