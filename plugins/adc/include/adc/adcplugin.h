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

#ifndef ADCPLUGIN_H
#define ADCPLUGIN_H

#define SCOPY_PLUGIN_NAME ADCPlugin

#include "src/adcacquisitionmanager.h"
#include "scopy-adc_export.h"

#include <iio.h>

#include <QLabel>
#include <QLineEdit>
#include <QObject>

#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include <gui/stylehelper.h>

#include <adcinstrumentcontroller.h>
namespace scopy {
namespace adc {

typedef enum
{
	TIME,
	FREQUENCY
} ADCInstrumentType;

class SCOPY_ADC_EXPORT ADCPlugin : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

	// Plugin interface
public:
	void initPreferences() override;
	void initMetadata() override;
	bool compatible(QString m_param, QString category) override;
	bool loadPreferencesPage() override;
	bool loadPage() override;
	bool loadIcon() override;
	void loadToolList() override;
	QString description() override;
	QString pkgName() override;
	QString about() override;
	QString version() override;
	bool onConnect() override;
	bool onDisconnect() override;
	void saveSettings(QSettings &) override;
	void loadSettings(QSettings &) override;

	void newInstrument(ADCInstrumentType t, AcqTreeNode *root);
	void deleteInstrument(ToolMenuEntry *w);

public Q_SLOTS:
	void preferenceChanged(QString, QVariant t1);

private:
	iio_context *m_ctx;
	QLineEdit *edit;
	QList<ADCInstrumentController *> m_ctrls;

	void createGRIIOTreeNode(TopBlockNode *node, iio_context *ctx);
};
} // namespace adc
} // namespace scopy

#endif // ADCPLUGIN_H
