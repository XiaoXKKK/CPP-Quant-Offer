import { chromium, devices } from '@playwright/test';
import { mkdirSync } from 'node:fs';
mkdirSync('.artifacts', { recursive: true });
const browser = await chromium.launch();
const desktop = await browser.newPage({ viewport: { width: 1440, height: 1050 } });
await desktop.goto('http://127.0.0.1:4321/CPP-Quant-Offer/');
await desktop.screenshot({ path: '.artifacts/desktop.png', fullPage: true });
await desktop.goto('http://127.0.0.1:4321/CPP-Quant-Offer/topics/cpp-memory-model/');
await desktop.screenshot({ path: '.artifacts/tutorial.png' });
const mobile = await browser.newPage({ ...devices['iPhone 13'], defaultBrowserType: undefined });
await mobile.goto('http://127.0.0.1:4321/CPP-Quant-Offer/');
await mobile.screenshot({ path: '.artifacts/mobile.png', fullPage: true });
await mobile.getByRole('button', { name: '目录', exact: true }).click();
await mobile.screenshot({ path: '.artifacts/mobile-menu.png' });
console.log(
  'Mobile menu bounding boxes',
  await mobile.locator('#sidebar').boundingBox(),
  await mobile.getByRole('link', { name: '01 专题索引' }).boundingBox(),
);
await browser.close();
