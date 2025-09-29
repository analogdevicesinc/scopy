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

#ifndef INITIALCALIBRATIONSWIDGET_H
#define INITIALCALIBRATIONSWIDGET_H

#include "scopy-adrv9002plugin_export.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>
#include <iio.h>
#include <iio-widgets/iiowidget.h>
#include <pluginbase/statusbarmanager.h>
#include <style.h>

namespace scopy::adrv9002 {

class SCOPY_ADRV9002PLUGIN_EXPORT InitialCalibrationsWidget : public QWidget
{
	Q_OBJECT

public:
	explicit InitialCalibrationsWidget(iio_device *device, QWidget *parent = nullptr);
	~InitialCalibrationsWidget();

	static bool isSupported(iio_device *device);

Q_SIGNALS:
	void calibrationStarted();
	void calibrationCompleted();
	void calibrationError(const QString &errorMessage);

public Q_SLOTS:
	void refreshStatus();

private Q_SLOTS:
	void onCalibrateNowClicked();

private:
	void setupUI();
	void connectSignals();
	QString getAttributeValue(const QString &attributeName);
	bool writeAttributeValue(const QString &attributeName, const QString &value);

	// Device communication
	iio_device *m_device;

	// UI components
	QLabel *m_titleLabel;
	QLabel *m_modeLabel;
	IIOWidget *m_modeComboWidget;
	QPushButton *m_calibrateBtn;
};

} // namespace scopy::adrv9002

#endif // INITIALCALIBRATIONSWIDGET_H