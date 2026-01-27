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

#include "scopy-adrv9009plugin_export.h"
#include <QWidget>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <gui/widgets/menusectionwidget.h>
#include <iio-widgets/iiowidget.h>
#include <iio.h>
#include <menuonoffswitch.h>
#include <menuspinbox.h>

namespace scopy::adrv9009 {

class SCOPY_ADRV9009PLUGIN_EXPORT BistWidget : public QWidget
{
	Q_OBJECT
public:
	BistWidget(iio_device *device, QWidget *parent = nullptr);
	~BistWidget();

Q_SIGNALS:
	void readRequested();

private slots:
	void onTxNcoControlChanged(); // Called when any TX NCO control changes
	void readTxNcoFromDevice();   // Called on refresh

private:
	iio_device *m_device;

	// TX NCO custom controls (mimic iio-oscilloscope pattern)
	MenuOnOffSwitch *m_txNcoEnable;
	gui::MenuSpinbox *m_tx1FreqSpinBox;
	gui::MenuSpinbox *m_tx2FreqSpinBox;

	void setupUi();
	QWidget *createBistSection(QWidget *parent);
	QWidget *createTxNcoSection(QWidget *parent);
	QWidget *createFramerPrbsSection(QWidget *parent);
	QWidget *createInitializeSection(QWidget *parent);

	// Helper methods for composite bist_tone attribute
	void writeBistToneToDevice();  // Write composite value to device
	void readBistToneFromDevice(); // Read and parse composite value
};

} // namespace scopy::adrv9009

#endif // BISTWIDGET_H
