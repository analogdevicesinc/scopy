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

#ifndef BISTWIDGET_H
#define BISTWIDGET_H

#include "scopy-ad9371plugin_export.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QList>
#include <iio.h>
#include <iio-widgets/iiowidget.h>
#include <gui/widgets/menuspinbox.h>

namespace scopy {
class IIOWidgetGroup;
}

namespace scopy::ad9371 {

class SCOPY_AD9371PLUGIN_EXPORT BistWidget : public QWidget
{
	Q_OBJECT

public:
	explicit BistWidget(iio_device *device, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~BistWidget();

Q_SIGNALS:
	void readRequested();

private Q_SLOTS:
	void onBistToneControlChanged();
	void onInitializeClicked();

private:
	void setupUi();
	QWidget *createBistControlsSection(QWidget *parent);
	void writeBistToneToDevice();
	void readBistToneFromDevice();

	iio_device *m_device;
	IIOWidgetGroup *m_widgetGroup = nullptr;
	QList<IIOWidget *> m_widgets;

	// BIST Tone custom controls
	QCheckBox *m_ncoEnableCheckbox;
	gui::MenuSpinbox *m_tx1NcoFreqSpin;
	gui::MenuSpinbox *m_tx2NcoFreqSpin;
};

} // namespace scopy::ad9371

#endif // BISTWIDGET_H
