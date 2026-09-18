import { test, expect } from '@playwright/test';
import { readFile } from 'node:fs/promises';

test('backup export and import merge state and preserve local drafts', async ({ page }) => {
  await page.goto('./');
  await page.locator('[data-topic-row="shared-mutex"] [data-progress="saved"]').click();
  await page.evaluate(() => {
    const key = 'cpp-quant-offer:study:v1';
    const state = JSON.parse(localStorage.getItem(key)!);
    state.drafts['shared-mutex-q01'] = 'local answer';
    localStorage.setItem(key, JSON.stringify(state));
  });
  await page.reload();
  await page.locator('#import-progress').setInputFiles({
    name: 'progress.json',
    mimeType: 'application/json',
    buffer: Buffer.from(
      JSON.stringify({
        version: 1,
        read: ['order-book'],
        saved: ['order-book'],
        drafts: { 'shared-mutex-q01': 'imported answer' },
        code: {},
      }),
    ),
  });
  await expect(page.locator('[data-topic-row="order-book"] [data-progress="read"]')).toHaveText(
    '✓ 已读',
  );
  if (await page.locator('.menu-button').isVisible()) await page.locator('.menu-button').click();
  const downloadPromise = page.waitForEvent('download');
  await page.locator('#export-progress').click();
  const download = await downloadPromise;
  const exported = JSON.parse(await readFile((await download.path())!, 'utf8'));
  expect(exported.saved.sort()).toEqual(['order-book', 'shared-mutex']);
  expect(exported.drafts['shared-mutex-q01']).toBe('local answer');
});

test('article and answers remain readable without JavaScript', async ({ browser }) => {
  const context = await browser.newContext({ javaScriptEnabled: false });
  const page = await context.newPage();
  const base = (process.env.BASE_PATH || '/CPP-Quant-Offer').replace(/\/$/, '');
  await page.goto(`http://127.0.0.1:4321${base}/topics/epoll-lt-et/`);
  await expect(page.locator('.prose')).toContainText('EAGAIN');
  const answer = page.locator('.answer').first();
  await answer.locator('summary').click();
  await expect(answer).toHaveAttribute('open');
  await context.close();
});
test('library supports full text, combined filters, empty state and URL restore', async ({
  page,
}) => {
  await page.goto('./');
  await expect(page.locator('.topic-row')).toHaveCount(5);
  await page.getByRole('searchbox').fill('coordinated omission');
  await expect(page.locator('.topic-row:visible')).toHaveCount(1);
  await expect(page.locator('.topic-row:visible')).toContainText('epoll');
  await page.reload();
  await expect(page.getByRole('searchbox')).toHaveValue('coordinated omission');
  await page.locator('#category').selectOption('concurrency');
  await expect(page.locator('#empty-state')).toBeVisible();
  await page.getByRole('button', { name: '清除筛选' }).click();
  await expect(page.locator('.topic-row:visible')).toHaveCount(5);
  await page.getByRole('searchbox').fill('epoll_create1');
  await expect(page.locator('.topic-row:visible')).toHaveCount(1);
  await page.getByRole('button', { name: '清除筛选' }).click();
  await page.locator('#level').selectOption('L1');
  await expect(page.locator('#empty-state')).toBeVisible();
});
test('read and saved state survive refresh; filters reflect state', async ({ page }) => {
  await page.goto('./');
  const row = page.locator('[data-topic-row="shared-mutex"]');
  await row.getByRole('button', { name: '☆ 收藏' }).click();
  await row.getByRole('button', { name: '标记已读' }).click();
  await page.reload();
  await expect(row.getByRole('button', { name: '★ 已收藏' })).toHaveAttribute(
    'aria-pressed',
    'true',
  );
  await page.locator('#status').selectOption('saved');
  await expect(page.locator('.topic-row:visible')).toHaveCount(1);
  await row.getByRole('button', { name: '★ 已收藏' }).click();
  await expect(page.locator('#empty-state')).toBeVisible();
});
test('tutorial renders tested code, hides answers and retains drafts', async ({ page }) => {
  await page.goto('./topics/cpp-memory-model/');
  await expect(page.locator('.prose pre').filter({ hasText: 'class Spsc' })).toBeVisible();
  const question = page.locator('.question').first();
  await expect(question.locator('.answer')).not.toHaveAttribute('open');
  await question.getByRole('textbox').fill('通过 release/acquire 建立 happens-before');
  await question.locator('summary').click();
  await expect(question.locator('.answer')).toHaveAttribute('open');
  await page.reload();
  await expect(question.getByRole('textbox')).toHaveValue(
    '通过 release/acquire 建立 happens-before',
  );
  await expect(question.locator('.answer')).not.toHaveAttribute('open');
});
test('random interview exhausts a filtered deck without repeats or answer leakage', async ({
  page,
}) => {
  await page.goto('./interview/');
  await page.locator('#interview-topic').selectOption('shared-mutex');
  await page.locator('#interview-level').selectOption('L1');
  await page.locator('#start-interview').click();
  const seen = new Set<string>();
  for (let i = 0; i < 5; i++) {
    await expect(page.locator('#interview-status')).toContainText(`第 ${i + 1} / 5`);
    const prompt = await page.locator('#question-prompt').innerText();
    expect(seen.has(prompt)).toBe(false);
    seen.add(prompt);
    await expect(page.locator('#interview-answer')).not.toHaveAttribute('open');
    await page.locator('#interview-answer summary').click();
    await page.locator('#next-question').click();
  }
  await expect(page.locator('#interview-status')).toContainText('已完成');
  await expect(page.locator('#next-question')).toBeDisabled();
});
test('code lab displays remote output and gracefully handles outage', async ({ page }) => {
  await page.route('https://godbolt.org/api/compiler/g132/compile', (route) =>
    route.fulfill({
      json: {
        code: 7,
        didExecute: true,
        buildResult: { code: 0 },
        stdout: [{ text: 'demo passed' }],
      },
    }),
  );
  await page.goto('./topics/shared-mutex/');
  await page.locator('#run-code').click();
  await expect(page.locator('#code-output')).toContainText('demo passed');
  await expect(page.locator('#code-output')).toContainText('运行退出码：7');
  await page.unrouteAll();
  await page.route('https://godbolt.org/api/compiler/g132/compile', (route) => route.abort());
  await page.locator('#code-editor').fill('int main() { return 0; }');
  await page.locator('#run-code').click();
  await expect(page.locator('#code-output')).toContainText('代码已保留');
  await expect(page.locator('#run-code')).toBeEnabled();
  await page.reload();
  await expect(page.locator('#code-editor')).toHaveValue('int main() { return 0; }');
});
test('responsive layout has no page overflow and navigation works', async ({ page }, testInfo) => {
  for (const route of ['./', './topics/order-book/', './roadmap/', './interview/']) {
    await page.goto(route);
    expect(await page.evaluate(() => document.documentElement.scrollWidth <= innerWidth + 1)).toBe(
      true,
    );
  }
  if (testInfo.project.name === 'mobile') {
    await page.getByRole('button', { name: '目录', exact: true }).click();
    await expect(page.locator('#sidebar')).toBeVisible();
    await page.getByRole('link', { name: '01 专题索引' }).click();
    await expect(page.getByRole('searchbox')).toBeVisible();
  }
});
test('storage failures preserve usability and imports reject malformed data', async ({ page }) => {
  await page.addInitScript(() => {
    Storage.prototype.setItem = () => {
      throw new DOMException('Quota', 'QuotaExceededError');
    };
  });
  await page.goto('./');
  await page.locator('[data-progress="saved"]').first().click();
  await expect(page.locator('#global-status')).toContainText('未能保存');
  await page.locator('#import-progress').setInputFiles({
    name: 'bad.json',
    mimeType: 'application/json',
    buffer: Buffer.from('{"version":2}'),
  });
  await expect(page.locator('#global-status')).toContainText('导入失败');
});
