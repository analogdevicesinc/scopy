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
// A "What's New" overlay pops on a fresh CI prefs file because the stored
// scopy_git_version never matches the build (scopymainwindow.cpp:464-466), and
// dont_show_whats_new does not suppress that branch. Tear the overlay down the
// same way the documentation screenshots flow does (screenshots.py first-run).
scopy.dismissWhatsNew();
msleep(300);
scopy.showPage("about");
msleep(500);
scopy.showAboutBuildInfo();
msleep(1000);
scopy.screenshot(scopyOutDir + "build-info.png");
scopy.exit();
