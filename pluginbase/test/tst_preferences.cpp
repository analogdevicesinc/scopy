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

#include <QDir>
#include <QList>
#include <QTest>

#include <pluginbase/preferences.h>

using namespace scopy;

class TST_Preferences : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void initPreferences();
	void getPreferences();

private:
	QSettings *s;
};

const QString filePath = QDir::currentPath() + "Pref.ini";

void TST_Preferences::initPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->setPreferencesFilename(filePath);
	p->init("test", "true");
	QString t = p->get("test").toString();
	QVERIFY("true" == t);
	p->save();
	p->clear();
}

void TST_Preferences::getPreferences()
{
	Preferences *p = Preferences::GetInstance();
	p->setPreferencesFilename(filePath);
	p->load();
	QString t = p->get("test").toString();
	QVERIFY("true" == t);
}

QTEST_MAIN(TST_Preferences)

#include "tst_preferences.moc"
