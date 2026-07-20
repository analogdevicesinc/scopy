// Build-info screenshot script.
// Expects `scopyOutDir` (absolute, trailing slash) to be injected by the launcher wrapper.
msleep(2000);
scopy.showPage("about");
msleep(500);
scopy.showAboutBuildInfo();
msleep(1000);
scopy.screenshot(scopyOutDir + "build-info.png");
scopy.exit();
