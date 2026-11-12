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

#include "fakestream.h"
#include "nodepanel.h"

#include <QtTest>

#include <component/attribute.h>
#include <component/device.h>
#include <menusectionwidget.h>

using namespace scopy;
using namespace scopy::component;
using namespace scopy::dcdebugger;
using namespace scopy::dcdebugger::test;

class TST_NodePanel : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void generalInfoStartsExpanded();
};

void TST_NodePanel::generalInfoStartsExpanded()
{
	// A node with attributes used to get its "General info" collapsed.
	Device dev;
	makeAttribute(&dev, "sampling_frequency", "1000");
	makeAttribute(&dev, "calibscale", "1");
	NodePanel *panel = createNodePanel(&dev, NodeKind::Device, "dev", {}, {});
	// "Attributes (2)" and "General info".
	const auto sections = panel->findChildren<MenuSectionCollapseWidget *>();
	QCOMPARE(sections.size(), 2);
	for(MenuSectionCollapseWidget *s : sections) {
		QVERIFY(!s->collapsed());
	}
	delete panel;
}

QTEST_MAIN(TST_NodePanel)
#include "tst_nodepanel.moc"
