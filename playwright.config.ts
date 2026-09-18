import { defineConfig, devices } from '@playwright/test';
const basePath = (process.env.BASE_PATH || '/CPP-Quant-Offer').replace(/\/$/, '');
const baseURL = `http://127.0.0.1:4321${basePath}/`;
export default defineConfig({
  testDir: './tests/e2e',
  fullyParallel: true,
  forbidOnly: !!process.env.CI,
  retries: process.env.CI ? 1 : 0,
  reporter: [['list'], ['html', { open: 'never' }]],
  use: { baseURL, trace: 'retain-on-failure' },
  projects: [
    { name: 'desktop', use: { ...devices['Desktop Chrome'] } },
    { name: 'mobile', use: { ...devices['iPhone 13'], defaultBrowserType: 'chromium' } },
  ],
  webServer: {
    command: 'npm run preview -- --port 4321',
    url: baseURL,
    reuseExistingServer: !process.env.CI,
  },
});
