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

#ifndef DEVICEIMPL_H
#define DEVICEIMPL_H

#include "device.h"
#include "pluginbase/plugin.h"
#include "pluginmanager.h"
#include "scopy-core_export.h"

#include <QCheckBox>
#include <QObject>
#include <QPushButton>

namespace scopy {

class SCOPY_CORE_EXPORT DeviceImpl : public QObject, public Device
{
	Q_OBJECT
public:
	explicit DeviceImpl(QString param, PluginManager *p, QString category = "", QObject *parent = nullptr);
	virtual ~DeviceImpl();

	// Device interface
public:
	QString id() override;
	QString displayName() override;
	QString displayParam() override;
	QString category() override;
	QString param() override;
	QWidget *icon() override;
	QWidget *page() override;
	QList<ToolMenuEntry *> toolList() override;
	virtual void init() override;
	virtual void preload() override;
	virtual void loadPlugins() override;
	virtual void unloadPlugins() override;
	virtual bool verify();
	virtual QMap<QString, QString> readDeviceInfo();

	QList<Plugin *> plugins() const;

public Q_SLOTS:
	virtual void connectDev() override;
	virtual void disconnectDev() override;
	virtual void showPage() override;
	virtual void hidePage() override;
	virtual void save(QSettings &) override;
	virtual void load(QSettings &) override;
	//	void forgetDev() override;
	void onConnectionFailed();
Q_SIGNALS:
	void toolListChanged() override;
	void connecting() override;
	void connected() override;
	void disconnected() override;
	void requestedRestart() override;
	void requestTool(QString) override;
	void connectionFailed();
	void forget();

protected:
	void removeDisabledPlugins();
	void loadName();
	void loadIcons();
	void loadPages();
	void loadToolList();
	void loadBadges();
	void setPingPlugin(Plugin *plugin);
	void bindPing();
	void unbindPing();

protected:
	PluginManager *p;
	QList<Plugin *> m_plugins;
	QList<Plugin *> m_connectedPlugins;
	QString m_id;
	QString m_category;
	QString m_displayName;
	QString m_displayParam;
	QString m_param;
	QWidget *m_icon;
	QWidget *m_page;
	QPushButton *connbtn, *discbtn;
	Plugin *m_pingPlugin = nullptr;
};
} // namespace scopy

#endif // DEVICEIMPL_H
