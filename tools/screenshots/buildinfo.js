/*
 * Copyright (c) 2026 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

// Build-info screenshot script.
// Expects `scopyOutDir` (absolute, trailing slash) to be injected by the launcher wrapper.
msleep(2000);
scopy.showPage("about");
msleep(500);
scopy.showAboutBuildInfo();
// Startup pops modal overlays that would obscure the build-info page:
//  - a GPL license dialog when general_first_run is still true (e.g. macOS,
//    where the seeded preferences.ini may not be the file Scopy reads);
//  - a "What's New" overlay because the stored scopy_git_version never matches
//    the freshly built version (scopymainwindow.cpp:464-466), which
//    dont_show_whats_new does not suppress.
// These are shown via queued connections during startup, so they can appear
// after an early dismiss. Dismiss them here, right before the screenshot, once
// all queued startup work has settled.
msleep(1000);
scopy.acceptLicense();
msleep(200);
scopy.dismissWhatsNew();
msleep(500);
scopy.screenshot(scopyOutDir + "build-info.png");
scopy.exit();
