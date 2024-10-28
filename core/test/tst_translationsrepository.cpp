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

#include "core/translationsrepository.h"

#include <QLibrary>
#include <QList>
#include <QTest>

using namespace scopy;

class TST_TranslationsRepository : public QObject
{
	Q_OBJECT
private Q_SLOTS:
	void checkPath();
	void checkGeneratedFiles();
	void loadTranslations();

private:
	QStringList libs;
};

void TST_TranslationsRepository::checkPath()
{
	QDir pathDir(TranslationsRepository::GetInstance()->getTranslationsPath());

	QVERIFY(pathDir.exists());
	QVERIFY(pathDir.entryList().contains("test.qm"));
	QVERIFY(pathDir.entryList().contains("test_test.qm"));
	qDebug() << "Found files:" << pathDir.entryList();
}

void TST_TranslationsRepository::checkGeneratedFiles()
{
	QStringList languages = TranslationsRepository::GetInstance()->getLanguages();
	qDebug() << "Found languages:" << languages;

	QVERIFY(languages.contains("test"));
}

void TST_TranslationsRepository::loadTranslations()
{
	QVERIFY(tr("TEST1") == "TEST1");
	QVERIFY(tr("TEST2") == "TEST2");

	TranslationsRepository::GetInstance()->loadTranslations("test");

	// verifying if test.qm was applied
	QVERIFY(tr("TEST1") == "test1");

	// verifying if test_test.qm was applied
	QVERIFY(tr("TEST2") == "test2");
}

QTEST_MAIN(TST_TranslationsRepository)

#include "tst_translationsrepository.moc"
