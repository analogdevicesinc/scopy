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
 */

#ifndef OBSSETTINGSWIDGET_H
#define OBSSETTINGSWIDGET_H

#include "scopy-ad9371plugin_export.h"
#include <QWidget>
#include <QVBoxLayout>
#include <iio.h>
#include <gui/widgets/menusectionwidget.h>
#include <menuonoffswitch.h>
#include <iio-widgets/iiowidget.h>

namespace scopy {
class IIOWidgetGroup;
}

namespace scopy::ad9371 {

class SCOPY_AD9371PLUGIN_EXPORT ObsSettingsWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ObsSettingsWidget(iio_device *device, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~ObsSettingsWidget();

Q_SIGNALS:
	void readRequested();

private:
	void setupUi();
	QWidget *createObsSettingsSection(QWidget *parent);
	QWidget *createObsProfileSection(QWidget *parent);
	QWidget *createSnifferProfileSection(QWidget *parent);

	void readChannelEnableFromDevice();
	void writeChannelEnableToDevice();

	iio_device *m_device;
	IIOWidgetGroup *m_widgetGroup = nullptr;

	// CHECKBOX_MASK bits for adi,obs-settings-obs-rx-channels-enable
	MenuOnOffSwitch *m_chEnable0 = nullptr;
	MenuOnOffSwitch *m_chEnable1 = nullptr;
	MenuOnOffSwitch *m_chEnable2 = nullptr;
	MenuOnOffSwitch *m_chEnable3 = nullptr;
	MenuOnOffSwitch *m_chEnable4 = nullptr;
};

} // namespace scopy::ad9371

#endif // OBSSETTINGSWIDGET_H
