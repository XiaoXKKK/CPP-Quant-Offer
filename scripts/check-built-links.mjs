import { readFileSync, existsSync } from 'node:fs';
import path from 'node:path';
import { load } from 'cheerio';
import { filesUnder } from './content.mjs';
const base = (process.env.BASE_PATH || '/CPP-Quant-Offer').replace(/\/$/, '');
const origin = process.env.SITE_URL || 'https://xiaoxkkk.github.io';
const outputDirectory = process.env.OUTPUT_DIR || 'dist';
const files = filesUnder(outputDirectory).filter((f) => f.endsWith('.html'));
const documents = new Map(files.map((f) => [path.resolve(f), load(readFileSync(f, 'utf8'))]));
const errors = [];
for (const [file, $] of documents) {
  const relative = path.relative(path.resolve(outputDirectory), file).replaceAll('\\', '/');
  const pageUrl = new URL(`${base}/${relative.replace(/index\.html$/, '')}`, origin);
  $('a[href],script[src],link[href],img[src]').each((_, element) => {
    const href = $(element).attr('href') || $(element).attr('src');
    if (!href || href.startsWith('mailto:') || href.startsWith('data:')) return;
    const url = new URL(href, pageUrl);
    if (url.origin !== pageUrl.origin) return;
    if (!url.pathname.startsWith(`${base}/`)) {
      errors.push(`${file}: escaped deployment base: ${href}`);
      return;
    }
    const localPath = decodeURIComponent(url.pathname.slice(base.length + 1));
    const target = path.resolve(
      outputDirectory,
      localPath.endsWith('/') || !localPath ? `${localPath}index.html` : localPath,
    );
    if (!existsSync(target)) {
      errors.push(`${file}: missing ${href}`);
      return;
    }
    if (url.hash && documents.has(target)) {
      const document = documents.get(target);
      const id = decodeURIComponent(url.hash.slice(1));
      if (
        !document('[id]')
          .toArray()
          .some((el) => document(el).attr('id') === id)
      )
        errors.push(`${file}: missing fragment ${href}`);
    }
  });
}
if (errors.length) {
  console.error(errors.join('\n'));
  process.exitCode = 1;
} else
  console.log(`Built links OK: ${files.length} HTML pages, including base path and fragments.`);
