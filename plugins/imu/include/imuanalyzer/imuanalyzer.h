/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef IMUANALYZER_H
#define IMUANALYZER_H

#define SCOPY_PLUGIN_NAME IMUAnalyzer

#include "scopy-imuanalyzer_export.h"
#include "imuanalyzerinterface.hpp"

#include <QLineEdit>
#include <QObject>

#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>
#include <toolbuttons.h>
#include <tooltemplate.h>
#include "qloggingcategory.h"

#include <pluginbase/plugin.h>
#include <pluginbase/pluginbase.h>
#include "style.h"

#include <iio.h>
#include <iioutil/connectionprovider.h>

namespace scopy {

class SCOPY_IMUANALYZER_EXPORT IMUAnalyzer : public QObject, public PluginBase
{
	Q_OBJECT
	SCOPY_PLUGIN;

public:
	void initMetadata() override;
	bool compatible(QString m_param, QString category) override;
	void loadToolList() override;

	bool onConnect() override;
	bool onDisconnect() override;
	bool loadIcon() override;

private:
	IMUAnalyzerInterface *m_imuInterface;
};

} // namespace scopy

#endif // IMUANALYZER
