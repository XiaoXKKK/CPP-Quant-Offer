import { readFileSync } from 'node:fs';
import { visit } from 'unist-util-visit';

// Include tested source at build time; authors never maintain a second code copy.
export function demoRemark() {
  return (tree) => {
    visit(tree, (node) => {
      if (!['link', 'definition'].includes(node.type)) return;
      if (/\.mdx?(?:#.*)?$/.test(node.url) && !/^https?:/.test(node.url)) {
        const [path, fragment] = node.url.split('#');
        const slug = path
          .split('/')
          .pop()
          .replace(/\.mdx?$/, '');
        node.url = `${(process.env.BASE_PATH || '/CPP-Quant-Offer').replace(/\/$/, '')}/topics/${slug}/${fragment ? `#${fragment}` : ''}`;
      }
    });
    visit(tree, 'code', (node) => {
      if (node.lang !== 'cpp' || !node.meta?.startsWith('include=')) return;
      const file = node.meta.slice('include='.length);
      if (!/^examples\/[a-z0-9-]+\.cpp$/.test(file)) throw new Error(`Unsafe demo path: ${file}`);
      node.value = readFileSync(file, 'utf8');
      node.meta = null;
    });
  };
}
