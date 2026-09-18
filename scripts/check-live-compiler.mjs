import { chromium } from '@playwright/test';
import assert from 'node:assert/strict';
const browser = await chromium.launch();
try {
  const page = await browser.newPage();
  const site = process.env.PREVIEW_URL || 'http://127.0.0.1:4321/CPP-Quant-Offer';
  await page.goto(`${site.replace(/\/$/, '')}/topics/shared-mutex/`);
  await page.locator('#run-code').click();
  await page.waitForFunction(() => !document.querySelector('#run-code').disabled, {
    timeout: 40000,
  });
  const output = await page.locator('#code-output').innerText();
  console.log(output);
  assert.match(output, /编译退出码：0/);
  assert.match(output, /运行退出码：0/);
  assert.match(output, /consistent snapshots/);
} finally {
  await browser.close();
}
