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

#ifndef TESTPLUGIN_H
#define TESTPLUGIN_H

#define SCOPY_PLUGIN_NAME TestPlugin

#include "scopy-testplugin_export.h"

#include <QLabel>
#include <QLineEdit>
#include <QObject>
#include <QPushButton>

#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy {

class SCOPY_TESTPLUGIN_EXPORT TestPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

	// Plugin interface
public:
	friend class TestPlugin_API;
	void initPreferences() override;
	void initMetadata() override;
	void init() override;
	bool compatible(QString m_param, QString category) override;
	bool loadPreferencesPage() override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	bool loadExtraButtons() override;
	QString about() override;
	QString version() override;
	bool onConnect() override;
	bool onDisconnect() override;
	void cloneExtra(Plugin *) override;
	void saveSettings(QSettings &) override;
	void loadSettings(QSettings &) override;

public Q_SLOTS:
	void messageCallback(QString topic, QString message) override;

private:
	QString m_initText;
	QWidget *tool;
	QWidget *tool2;
	QLineEdit *edit;
	int renameCnt;
	ApiObject *m_pluginApi;
	void startTutorial();
	void initHoverWidgetTests();

	QLabel *lbl;
	QLabel *pic;
	QLabel *lbl2;
	QPushButton *btn;
	QPushButton *btn2;
	QPushButton *btn3;
	QPushButton *btn4;
};

class SCOPY_TESTPLUGIN_EXPORT TestPlugin_API : public ApiObject
{
	Q_OBJECT
public:
	explicit TestPlugin_API(TestPlugin *p)
		: ApiObject(p)
		, p(p)
	{}
	~TestPlugin_API() {}
	TestPlugin *p;

	Q_PROPERTY(QString testText READ testText WRITE setTestText);
	QString testText() const;
	void setTestText(const QString &newTestText);

private:
	QString m_testText;
};

} // namespace scopy

#endif // TESTPLUGIN_H
