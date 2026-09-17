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
#include "pkg-manager/pkgmanager.h"

#include <component/device.h>
#include <component/channel.h>
#include <component/attribute.h>
#include <component/attributewriter.h>
#include <component/navigation.h>

#include <QFileDialog>
#include <QLabel>
#include <QDebug>
#include <QCoreApplication>
#include <style.h>
#include <qloggingcategory.h>
#include <pluginbase/preferences.h>
#include <scopy-ad936x_config.h>

using namespace scopy;
using namespace ad936x;
Q_LOGGING_CATEGORY(CAT_FIR_FILTER, "FirFilter")

FirFilterQWidget::FirFilterQWidget(component::Device *dev1, component::Device *dev2, QWidget *parent)
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
	QFileInfoList files = PkgManager::listFilesInfo(QStringList() << "ad936x-filters");
	QString defaultDir = (files.isEmpty()) ? PkgManager::packagesPath() : files.first().absolutePath();

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
		if(auto *a = component::attributeByName(m_dev1, "filter_fir_config"); a && a->writeCapability()) {
			a->writeCapability()->writeAsync(buffer);
		}

		if(m_dev2 != nullptr) {
			if(auto *a = component::attributeByName(m_dev2, "filter_fir_config");
			   a && a->writeCapability()) {
				a->writeCapability()->writeAsync(buffer);
			}
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
	component::Channel *chn = component::channelById(m_dev1, "voltage0", isTx);
	toggleChannelFilter(chn, "filter_fir_en", toggled);
	if(m_dev2 != nullptr) {
		component::Channel *chn = component::channelById(m_dev2, "voltage0", isTx);
		toggleChannelFilter(chn, "filter_fir_en", toggled);
	}
}

void FirFilterQWidget::toggleDeviceFilter(component::Device *dev, bool toggled)
{
	// for FMCOMMS2 devices the attribute might be a device attr or a channel attr
	if(auto *a = component::attributeByName(dev, "in_out_voltage_filter_fir_en"); a && a->writeCapability()) {
		a->writeCapability()->writeAsync(toggled ? "1" : "0");
		Q_EMIT filterChanged();
	} else {
		// if it is not a device attr then it should be a channel attr for channel out
		component::Channel *chn = component::channelById(dev, "out", false);
		toggleChannelFilter(chn, "voltage_filter_fir_en", toggled);
	}
}

void FirFilterQWidget::toggleChannelFilter(component::Channel *chn, QString attr, bool toggled)
{
	if(chn) {
		if(auto *a = component::attributeByName(chn, attr); a && a->writeCapability()) {
			a->writeCapability()->writeAsync(toggled ? "1" : "0");
			Q_EMIT filterChanged();
		}
	}
}
