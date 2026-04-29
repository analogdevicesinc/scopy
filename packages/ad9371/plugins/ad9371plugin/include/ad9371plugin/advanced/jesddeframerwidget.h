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

#ifndef JESDDEFRAMERWIDGET_H
#define JESDDEFRAMERWIDGET_H

#include "scopy-ad9371plugin_export.h"
#include <QWidget>
#include <QVBoxLayout>
#include <iio.h>
#include <gui/widgets/menusectionwidget.h>
#include <menuonoffswitch.h>

namespace scopy {
class IIOWidgetGroup;
}

namespace scopy::ad9371 {

class SCOPY_AD9371PLUGIN_EXPORT JesdDeframerWidget : public QWidget
{
	Q_OBJECT

public:
	explicit JesdDeframerWidget(iio_device *device, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~JesdDeframerWidget();

Q_SIGNALS:
	void readRequested();

private:
	void setupUi();

	iio_device *m_device;
	IIOWidgetGroup *m_widgetGroup = nullptr;
};

} // namespace scopy::ad9371

#endif // JESDDEFRAMERWIDGET_H
