import { readTopics } from './content.mjs';
const urls = new Set(
  readTopics().flatMap((t) => [
    ...t.data.references.map((r) => r.url),
    ...t.data.questions
      .filter((q) => q.source.kind === 'public-interview')
      .map((q) => q.source.url),
  ]),
);
let failed = 0;
for (const url of urls) {
  try {
    const response = await fetch(url, {
      signal: AbortSignal.timeout(20000),
      headers: { 'User-Agent': 'CPP-Quant-Offer reference checker' },
    });
    await response.body?.cancel();
    console.log(`${response.status} ${url}`);
    if (!response.ok) failed++;
  } catch (error) {
    console.error(`${url}: ${error.message}`);
    failed++;
  }
}
if (failed) {
  console.error(
    `${failed} references need review; rate limits and anti-bot responses require manual verification.`,
  );
  process.exitCode = 1;
}
