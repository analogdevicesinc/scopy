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

#ifndef ACQUISITIONPLUGIN_H
#define ACQUISITIONPLUGIN_H

#define SCOPY_PLUGIN_NAME AcquisitionPlugin

#include "scopy-acquisition_export.h"

#include <QObject>

#include <component/controller.h>
#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>

namespace scopy::adc {
class AcqInstrumentController;
}

namespace scopy::acquisition {

// A device-agnostic plugin built entirely on the device controller: it never
// touches libiio or ConnectionProvider, only component::Controller and the
// component tree hanging off the Context. It contributes the device page, which
// lists the context's identity and attributes as read through the component
// capabilities, and the Acquisition instrument.
class SCOPY_ACQUISITION_EXPORT AcquisitionPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	bool compatible(QString param, QString category) override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	void unload() override;
	void newInstrument();
	void deleteInstrument(ToolMenuEntry *w);
	void initMetadata() override;
	QString description() override;
	QString pkgName() override;
	QString about() override;
	QString version() override;

public Q_SLOTS:
	bool onConnect() override;
	bool onDisconnect() override;

private:
	// The device-controller context for m_param, refcounted. Held for the whole
	// connected lifetime and released last on disconnect: every source block the
	// instrument builds is parented inside it and still holds the devices and
	// streams hanging off the Context.
	component::ContextHandle m_context;
	QList<adc::AcqInstrumentController *> m_ctrls;
};

} // namespace scopy::acquisition
#endif // ACQUISITIONPLUGIN_H
