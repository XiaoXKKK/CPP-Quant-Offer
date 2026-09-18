import { readFileSync } from 'node:fs';
export function GET() {
  return new Response(readFileSync('schemas/topic.schema.json', 'utf8'), {
    headers: { 'Content-Type': 'application/schema+json' },
  });
}
