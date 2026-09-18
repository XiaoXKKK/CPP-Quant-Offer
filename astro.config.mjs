import { defineConfig } from 'astro/config';
import mdx from '@astrojs/mdx';
import { unified } from '@astrojs/markdown-remark';
import { demoRemark } from './scripts/demo-remark.mjs';

export default defineConfig({
  site: process.env.SITE_URL || 'https://xiaoxkkk.github.io',
  base: process.env.BASE_PATH || '/CPP-Quant-Offer',
  output: 'static',
  trailingSlash: 'always',
  integrations: [mdx()],
  markdown: {
    processor: unified({ remarkPlugins: [demoRemark] }),
    shikiConfig: { theme: 'github-dark' },
  },
  devToolbar: { enabled: false },
});
