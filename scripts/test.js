const puppeteer = require('puppeteer');
(async () => {
    const browser = await puppeteer.launch();
    const page = await browser.newPage();
    page.on('console', msg => console.log('PAGE LOG:', msg.text()));
    page.on('pageerror', err => console.log('PAGE ERROR:', err.toString()));
    await page.goto('http://localhost:3000');
    // click PLAY
    await page.evaluate(() => { showModeMenu(); selectMode(0); showLoadoutMenu(1); startGameActual(); });
    await page.waitForTimeout(1000);
    await browser.close();
})();
