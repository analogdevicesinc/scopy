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
#include "scopy-core_export.h"

#include <QCheckBox>
#include <QObject>
#include <QPushButton>
#include <QTabWidget>

#include <component/controller.h>

#include <map>

namespace scopy {
class ConnectionLostWidget;

class SCOPY_CORE_EXPORT DeviceImpl : public QObject, public Device
{
	Q_OBJECT
public:
	// A non-empty kind makes this a connection-backed device: the connection and every
	// compatible view are opened on construction (so plugins can inspect them in compatible()),
	// released when the device goes idle, and on every connect only the views the loaded plugins
	// declare through their "category" are rebuilt.
	explicit DeviceImpl(QString param, QString category = "", component::ConnectionKind kind = {},
			    int maxThreads = 1, QObject *parent = nullptr);
	virtual ~DeviceImpl();

	// Device interface
public:
	QString id() override;
	QString displayName() override;
	QString displayParam() override;
	QString category() override;
	QString param() override;
	QWidget *icon() override;
	QPixmap iconPixmap() override;
	QWidget *page() override;
	QWidget *configPage() override;
	QList<ToolMenuEntry *> toolList() override;
	virtual void init() override;
	virtual void preload() override;
	virtual void loadPlugins() override;
	virtual void unloadPlugins() override;
	virtual bool verify();
	virtual QMap<QString, QString> readDeviceInfo();

	QList<Plugin *> plugins() const;
	DeviceImpl::DeviceState_t state() override;
	Plugin *getPluginByName(const QString &pluginName);

	component::Connection *connection() const;
	// Compatible backends of the connection; empty for a connection-less device.
	QStringList backends() const;
	// A held view; an empty backend means "iio", or the first held view if there is none.
	component::Context *context(const component::BackendId &backend = {}) const;

public Q_SLOTS:
	virtual void connectDev() override;
	virtual void disconnectDev() override;
	virtual void showPage() override;
	virtual void hidePage() override;
	virtual void save(QSettings &) override;
	virtual void load(QSettings &) override;
	//	void forgetDev() override;
	void onConnectionFailed();
	void onConnectionLost();
Q_SIGNALS:
	void toolListChanged() override;
	void connecting() override;
	void connected() override;
	void disconnecting() override;
	void disconnected() override;
	void requestedRestart() override;
	void requestTool(QString) override;
	void connectionFailed();
	void forget();
	void requestReload(QString id, QStringList plugins);

protected:
	void removeDisabledPlugins();
	void loadName();
	void loadIcons();
	void loadPages();
	void loadConfigPage();
	void loadToolList();
	void loadBadges();
	void loadCompatiblePluginsTab(QWidget *pluginsTab);
	QStringList getPluginsName();
	void setState(DeviceState_t state);
	void acquireConnection();
	void acquireViews(const QStringList &backends);
	void releaseContext();
	QStringList declaredBackends() const;

private:
	void onStateChanged(DeviceState_t state);

Q_SIGNALS:
	void stateChanged(scopy::Device::DeviceState_t state);

protected:
	QList<Plugin *> m_plugins;
	QList<Plugin *> m_connectedPlugins;
	QSet<QString> m_reloadPluginsSet;
	DeviceState_t m_state = DEV_INIT;
	QString m_id;
	QString m_category;
	QString m_displayName;
	QString m_displayParam;
	QString m_param;
	QWidget *m_icon;
	QWidget *m_page;
	QTabWidget *m_configPage;
	QPushButton *connbtn, *discbtn;
	ConnectionLostWidget *m_connectionLostWidget = nullptr;
	component::ConnectionKind m_kind;
	int m_maxThreads = 1;
	component::ConnectionHandle m_conn;
	std::map<component::BackendId, component::ContextHandle> m_views;
};
} // namespace scopy

#endif // DEVICEIMPL_H
