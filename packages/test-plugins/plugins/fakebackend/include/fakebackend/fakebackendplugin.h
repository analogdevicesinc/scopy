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

#ifndef FAKEBACKENDPLUGIN_H
#define FAKEBACKENDPLUGIN_H

#define SCOPY_PLUGIN_NAME FakeBackendPlugin

#include "scopy-fakebackend_export.h"

#include <QObject>

#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

class QLabel;

namespace scopy {

// End-to-end check of runtime registration: init() registers FakeProvider + FakeCtxBackend,
// auto-scan finds the devices, they are created with category "fake", and this plugin is matched
// by category (which also names the view it uses).
// The number of fake devices is FAKE_DEVICE_COUNT in fakebackendplugin.cpp (0 disables it).
class SCOPY_FAKEBACKEND_EXPORT FakeBackendPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	void initMetadata() override;
	void init() override;
	bool compatible(QString m_param, QString category) override;
	void loadToolList() override;

	bool onConnect() override;
	bool onDisconnect() override;

private:
	void refresh();

	QLabel *m_label = nullptr;
};

} // namespace scopy

#endif // FAKEBACKENDPLUGIN_H
