/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "core/pluginrepository.h"

#include <QLibrary>
#include <QList>
#include <QTest>

using namespace scopy;

class TST_PluginRepository : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void loadLibs();

private:
	QStringList libs;
};

#define NONPLUGIN_LIBRARY_LOCATION "../libscopycore.so"
#define PLUGIN_LOCATION "../../plugins/plugins"

void TST_PluginRepository::loadLibs()
{
	PluginRepository *p = new PluginRepository(this);
	PluginManager *pm = p->getPluginManager();
	QVERIFY(pm->metadata().isEmpty());
	p->init(PLUGIN_LOCATION);
	//	QVERIFY(!pm->metadata().isEmpty());

	delete p;
}

QTEST_MAIN(TST_PluginRepository)

#include "tst_pluginrepository.moc"
