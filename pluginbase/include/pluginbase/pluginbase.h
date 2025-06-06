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

#ifndef PLUGINBASE_H
#define PLUGINBASE_H
#include "apiobject.h"
#include "plugin.h"
#include "iioutil/pingtask.h"
#include "scopy-pluginbase_export.h"

#include <QObject>

namespace scopy {
/**
 * @brief The PluginBase class
 * PluginBase is used to provide default implementation to the plugin interface
 */
class SCOPY_PLUGINBASE_EXPORT PluginBase : public Plugin
{

public:
	virtual ~PluginBase() {}

	virtual void setParam(QString param, QString category) override;
	virtual void setEnabled(bool en) override;
	virtual void initMetadata() override;
	virtual void setMetadata(QJsonObject obj) override;
	virtual void initPreferences() override;
	virtual void init() override;
	virtual void deinit() override;
	virtual void preload() override;
	virtual void postload() override;

	virtual bool loadIcon() override;
	virtual bool loadPage() override;
	virtual bool loadConfigPage() override;
	virtual void loadToolList() override;
	virtual bool loadPreferencesPage() override;
	virtual bool loadExtraButtons() override;

	virtual void saveSettings(QSettings &) override;
	virtual void loadSettings(QSettings &) override;

	virtual void unload() override;

	virtual QString param() override;
	virtual bool enabled() override;
	virtual QString name() override;
	virtual QString pkgName() override;
	virtual QString displayName() override;
	virtual QString displayParam() override;
	virtual QWidget *icon() override;
	virtual QWidget *page() override;
	virtual QWidget *configPage() override;
	virtual QWidget *preferencesPage() override;
	virtual QList<QAbstractButton *> extraButtons() override;
	virtual QList<ToolMenuEntry *> toolList() override;
	virtual QJsonObject metadata() override;

	virtual QString about() override;
	virtual QString version() override;
	virtual QString description() override;

	virtual void loadMetadata(QString data);
	virtual void cloneExtra(Plugin *) override;

	virtual PingTask *pingTask() override;

public Q_SLOTS:
	virtual void showPageCallback() override;
	virtual void hidePageCallback() override;
	virtual void messageCallback(QString topic, QString message) override;
	virtual void requestTool(QString);
	virtual void startPingTask() override;
	virtual void stopPingTask() override;
	virtual void onPausePingTask(bool) override;

protected:
	QString m_param;
	QString m_category;
	QString m_name;
	QString m_displayName;
	QString m_displayParam;
	QWidget *m_page;
	QWidget *m_configPage;
	QWidget *m_preferencesPage;
	QWidget *m_icon;
	QList<ToolMenuEntry *> m_toolList;
	QList<QAbstractButton *> m_extraButtons;
	QJsonObject m_metadata;
	bool m_enabled;
	PingTask *m_pingTask = nullptr;
	CyclicalTask *m_cyclicalTask = nullptr;
};
} // namespace scopy

#define scopyxstr(a) scopystr(a)
#define scopystr(a) #a

#define SCOPY_PLUGIN                                                                                                   \
	Q_PLUGIN_METADATA(IID ScopyPlugin_iid)                                                                         \
	Q_INTERFACES(scopy::Plugin)                                                                                    \
public:                                                                                                                \
	virtual ~SCOPY_PLUGIN_NAME() override {}                                                                       \
	SCOPY_PLUGIN_NAME *clone(QObject *parent) override                                                             \
	{                                                                                                              \
		SCOPY_PLUGIN_NAME *ret = new SCOPY_PLUGIN_NAME();                                                      \
		/* copy metadata from this object to the next one */                                                   \
		ret->m_name = scopyxstr(SCOPY_PLUGIN_NAME);                                                            \
		ret->setParent(parent);                                                                                \
		ret->m_displayName = ret->m_name;                                                                      \
		ret->setMetadata(metadata());                                                                          \
		ret->cloneExtra(this);                                                                                 \
		return ret;                                                                                            \
	}                                                                                                              \
                                                                                                                       \
Q_SIGNALS:                                                                                                             \
	void connectDevice() override;                                                                                 \
	void disconnectDevice() override;                                                                              \
	void restartDevice() override;                                                                                 \
	void toolListChanged() override;                                                                               \
	void requestToolByUuid(QString) override;                                                                      \
	void pausePingTask(bool) override;                                                                             \
                                                                                                                       \
private:

#define SCOPY_PLUGIN_ICON(x)                                                                                           \
	m_icon = new QLabel("");                                                                                       \
	m_icon->setStyleSheet("border-image: url(" x ");")
#define SCOPY_NEW_TOOLMENUENTRY(id, name, icon) new ToolMenuEntry(id, name, icon, this->m_name, this->m_param, this)

#endif // PLUGINBASE_H
