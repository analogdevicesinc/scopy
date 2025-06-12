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

#include "firfilterqwidget.h"

#include <QFileDialog>
#include <QLabel>
#include <QDebug>
#include <QCoreApplication>
#include <style.h>
#include <qloggingcategory.h>
#include <pluginbase/preferences.h>
#include <scopy-pluto_config.h>

using namespace scopy;
using namespace pluto;
Q_LOGGING_CATEGORY(CAT_FIR_FILTER, "FirFilter")

FirFilterQWidget::FirFilterQWidget(iio_device *dev1, iio_device *dev2, QWidget *parent)
	: m_dev1(dev1)
	, m_dev2(dev2)
	, QWidget{parent}
{

	m_layout = new QVBoxLayout(this);
	setLayout(m_layout);

	m_isRxFilter = false;
	m_isTxFilter = false;

	QLabel *label = new QLabel("Filter FIR configuration:", this);

	m_chooseFileBtn = new QPushButton("(None)", this);

	Style::setStyle(m_chooseFileBtn, style::properties::button::basicButton);

	connect(m_chooseFileBtn, &QPushButton::clicked, this, &FirFilterQWidget::chooseFile);

	MenuOnOffSwitch *autoFilter = new MenuOnOffSwitch("Auto Filter", this, false);

	connect(autoFilter->onOffswitch(), &QAbstractButton::toggled, this, &FirFilterQWidget::autofilterToggled);

	m_applyRxTxFilter = new MenuOnOffSwitch("Enable RX and TX FIR Filters", this, false);

	connect(m_applyRxTxFilter->onOffswitch(), &QAbstractButton::toggled, this, [=, this](bool toggled) {
		if(m_isRxFilter) {
			if(m_isTxFilter) {
				// we have a rx & tx filter we try to apply filter for device
				toggleDeviceFilter(m_dev1, toggled);
				if(m_dev2 != nullptr) {
					toggleDeviceFilter(m_dev2, toggled);
				}
			} else {
				// we have a Rx filter
				applyChannelFilterToggled(false, toggled);
			}

		} else if(m_isTxFilter) {
			// we have a Tx filter
			applyChannelFilterToggled(true, toggled);
		}
	});

	m_layout->addWidget(label);
	m_layout->addWidget(autoFilter);
	m_layout->addWidget(m_chooseFileBtn);
	m_layout->addWidget(m_applyRxTxFilter);

	m_applyRxTxFilter->setVisible(false);
}

void FirFilterQWidget::chooseFile()
{
	// Determine default search dir
	QString defaultDir = AD936X_FILTERS_BUILD_PATH;
#ifdef Q_OS_WINDOWS
	defaultDir = AD936X_FILTERS_PATH_LOCAL;
#elif defined __APPLE__
	defaultDir = QCoreApplication::applicationDirPath() + "/plugins/ad936x";
#elif defined(__appimage__)
	defaultDir = QCoreApplication::applicationDirPath() + "/../lib/scopy/plugins/ad936x";
#else
	defaultDir = AD936X_FILTERS_SYSTEM_PATH;
#endif

	bool useNativeDialogs = Preferences::get("general_use_native_dialogs").toBool();
	QString selectedFilter;
	// Is there a way to open file dialog at our asset file ?
	QString filename = QFileDialog::getOpenFileName(
		this, tr("Export"), defaultDir, tr("Filter Files (*.ftr);;All Files(*)"), &selectedFilter,
		(useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	if(!filename.isEmpty()) {
		m_chooseFileBtn->setText(filename);
		applyFirFilter(filename);
	}
}

void FirFilterQWidget::applyFirFilter(QString path)
{
	if(path.isEmpty()) {
		qWarning(CAT_FIR_FILTER) << "No file path provided";
		return;
	}

	QFile file(path);

	bool rx = false, tx = false;

	if(file.open(QIODevice::ReadOnly)) {

		QTextStream textStream(&file);
		auto line = textStream.readLine();

		int ret = -ENOMEM;

		// Skip comment lines
		while(!textStream.atEnd()) {
			line = textStream.readLine();

			// Determine RX or TX
			if(line.contains("RX", Qt::CaseInsensitive)) {
				rx = true;
			} else if(line.contains("TX", Qt::CaseInsensitive)) {
				tx = true;
			}

			// if we got both rx and tx we can stop searching
			if(rx && tx)
				break;
		}

		// Read the entire file content
		textStream.seek(0);
		QString buffer = textStream.readAll();
		file.close();

		// Write configuration to the device(s)
		ret = iio_device_attr_write_raw(m_dev1, "filter_fir_config", buffer.toStdString().c_str(),
						buffer.length());

		if(m_dev2 != nullptr) {
			int ret2 = iio_device_attr_write_raw(m_dev2, "filter_fir_config", buffer.data(), buffer.size());
			ret = std::min(ret, ret2);
		}

		if(ret < 0) {
			qWarning(CAT_FIR_FILTER)
				<< "FIR Filter Configuration Failed" << QString::fromLocal8Bit(strerror(ret * (-1)));
		}
	} else {
		qWarning(CAT_FIR_FILTER) << "Unable to open file: " << path;
		return;
	}

	if(rx || tx) {
		m_isRxFilter = rx;
		m_isTxFilter = tx;
		m_applyRxTxFilter->setVisible(true);
	}
}

void FirFilterQWidget::applyChannelFilterToggled(bool isTx, bool toggled)
{
	iio_channel *chn = iio_device_find_channel(m_dev1, "voltage0", isTx);
	toggleChannelFilter(chn, "filter_fir_en", toggled);
	if(m_dev2 != nullptr) {
		iio_channel *chn = iio_device_find_channel(m_dev2, "voltage0", isTx);
		toggleChannelFilter(chn, "filter_fir_en", toggled);
	}
}

void FirFilterQWidget::toggleDeviceFilter(iio_device *dev, bool toggled)
{
	int ret = -ENOMEM;
	// for FMCOMMS2 devices the attribute might be a device attr or a channel attr
	ret = iio_device_attr_write_bool(dev, "in_out_voltage_filter_fir_en", toggled);

	// if you are not able to write it as a device attr then it should be a channel attr for channel out
	if(ret < 0) {
		iio_channel *chn = iio_device_find_channel(dev, "out", false);
		toggleChannelFilter(chn, "voltage_filter_fir_en", toggled);
	} else {
		Q_EMIT filterChanged();
	}
}

void FirFilterQWidget::toggleChannelFilter(iio_channel *chn, QString attr, bool toggled)
{
	if(chn) {
		int ret = -ENOMEM;
		ret = iio_channel_attr_write_bool(chn, attr.toStdString().c_str(), toggled);
		if(ret < 0) {
			qWarning(CAT_FIR_FILTER) << "Failed to enable FIR filter channel: "
						 << QString::fromLocal8Bit(strerror(ret * (-1)));
		} else {
			Q_EMIT filterChanged();
		}
	}
}
