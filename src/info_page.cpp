/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "info_page.hpp"
#include "ui_info_page.h"
#include "preferences.h"
#include <libm2k/m2k.hpp>
#include <libm2k/contextbuilder.hpp>
#include "libm2k/analog/dmm.hpp"
#include "dynamicWidget.hpp"
#include <QString>
#include <QTimer>
#include <QtConcurrentRun>

using namespace std;
using namespace adiscope;

InfoPage::InfoPage(QString uri, Preferences *pref, PhoneHome* phoneHome,
		   struct iio_context *ctx,
		   QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InfoPage),
	m_uri(uri),
	m_ctx(ctx),
	m_advanced(false),
	prefPanel(pref),
	m_led_timer(new QTimer(this)),
	m_blink_timer(new QTimer(this)),
	m_connected(false),
	m_search_interrupted(false),
	m_phoneHome(phoneHome)
{
	ui->setupUi(this);
	ui->paramLabel->setText(uri);

	if (uri.startsWith("usb:")) {
		ui->btnForget->setEnabled(false);
	}

	ui->btnCalibrate->setEnabled(false);
	ui->btnCalibrate->setVisible(false);
	connect(prefPanel, &Preferences::notify, this, &InfoPage::readPreferences);
	connect(ui->btnIdentify, SIGNAL(pressed()),
		this, SLOT(identifyDevice()));
	connect(m_led_timer, SIGNAL(timeout()),
		this, SLOT(ledTimeout()));
	connect(m_blink_timer, SIGNAL(timeout()),
		this, SLOT(blinkTimeout()));
	connect(m_phoneHome, &PhoneHome::m2kVersionChanged, this, [=] {
		const int checked = dynamic_cast<M2kInfoPage*>(this)->checkLatestFwVersion(m_info_params.value("Firmware version"));
		if (checked == 1) {
			ui->lblFirmware->setText("Firmware is up to date!");
		} else if (checked == 0) {
			const QString message = "New firmware version is available. "
							  "Download " + m_phoneHome->getM2kVersion() + " "
							  "<a href=\"";
			ui->lblFirmware->setText(message + m_phoneHome->getM2kLink() + "\">here</a>");
			ui->lblFirmware->setTextFormat(Qt::RichText);
			ui->lblFirmware->setTextInteractionFlags(Qt::TextBrowserInteraction);
			ui->lblFirmware->setOpenExternalLinks(true);
		}
	});
	readPreferences();
	setStatusLabel(ui->lblCalibrationInfo);
	setStatusLabel(ui->lblCalibrationStatus);
	setStatusLabel(ui->lblConnectionStatus);
	setStatusLabel(ui->lblIdentifyStatus);
}

void InfoPage::readPreferences() {
	m_advanced = prefPanel->getAdvanced_device_info();
	getDeviceInfo();
}

InfoPage::~InfoPage()
{
	if (m_ctx) {
		m_ctx = nullptr;
	}
	delete ui;
}

struct iio_context *InfoPage::ctx() const
{
	return m_ctx;
}

void InfoPage::setCtx(struct iio_context *ctx)
{
	identifyDevice(false);
	(!ctx) ? m_connected = false : m_connected = true;
	m_ctx = ctx;
}

void InfoPage::getDeviceInfo()
{
	struct iio_context* temp_ctx = m_ctx;
	if (!m_ctx) {
		temp_ctx = iio_create_context_from_uri(m_uri.toStdString().c_str());
	}

	std::string str = "";
	if (temp_ctx) {
		const char *name;
		const char *value;
		char ctx_git_tag[8];
		unsigned int ctx_major, ctx_minor;
		iio_context_get_version(temp_ctx, &ctx_major, &ctx_minor, ctx_git_tag);

		m_info_params.insert("IIO version",
				     QString::fromStdString(to_string(ctx_major) + "." +
							    to_string(ctx_minor)));

		QString description(iio_context_get_description(temp_ctx));
		m_info_params.insert("Linux", description);

		int attr_no = iio_context_get_attrs_count(temp_ctx);
		for (int i = 0; i < attr_no; i++) {
			if (!iio_context_get_attr(temp_ctx, i, &name, &value)) {
				auto pair = translateInfoParams(QString::fromUtf8(name));
				if (pair.second == "")
					continue;
				if (pair.first) {
					m_info_params_advanced.insert(pair.second, QString::fromUtf8(value));
				} else {
					m_info_params.insert(pair.second, QString::fromUtf8(value));
				}
			}
		}

		if (!m_ctx) {
			iio_context_destroy(temp_ctx);
			temp_ctx = nullptr;
		}
	}

	ui->btnIdentify->setEnabled(supportsIdentification());
	refreshInfoWidget();
}

QPair<bool, QString> InfoPage::translateInfoParams(QString key)
{
	bool advanced = false;
	if (key.contains("fw_version")) {
		key = "Firmware version";
	} else if (key == "hw_model") {
		key = "Model";
	} else if (key.contains("hw_model_variant")) {
		key = "Model variant";
		advanced = true;
	} else if (key.contains("serial")) {
		key = "Serial";
	} else if (key.contains("local,kernel")) {
		key = "";
	} else if (key.contains("idProduct")) {
		key = "Product ID";
		advanced = true;
	} else if (key.contains("idVendor")) {
		key = "Vendor ID";
		advanced = true;
	} else if (key.contains("usb,product")) {
		key = "Product name";
	} else if (key.contains("usb,vendor")) {
		key = "Vendor";
	} else if (key.contains("release")) {
		key = "Release";
		advanced = true;
	} else if (key.contains("ip")) {
		key = "IP Address";
	} else if (key.startsWith("cal,gain") ||
		   key.startsWith("cal,offset") ||
		   key.startsWith("cal,temp_lut")) {
		advanced = true;
	}
	return QPair<bool, QString>(advanced, key);
}

void InfoPage::setCalibrationStatusLabel(QString str, QString color)
{
	setStatusLabel(ui->lblCalibrationStatus,str,color);
}

void InfoPage::setCalibrationInfoLabel(QString str, QString color)
{
	setStatusLabel(ui->lblCalibrationInfo,str,color);
}

void InfoPage::setIdentyifyLabel(QString str, QString color)
{
	setStatusLabel(ui->lblIdentifyStatus,str,color);
}

void InfoPage::setConnectionStatusLabel(QString str, QString color)
{
	setStatusLabel(ui->lblConnectionStatus,str,color);
}

void InfoPage::setStatusLabel(QLabel *lbl, QString str, QString color)
{
	if(!str.isEmpty())
	{
		lbl->setVisible(true);
		lbl->setText(str);

		if(color == "white"){
			setDynamicProperty(lbl, "invalid", false);
			setDynamicProperty(lbl, "valid", true);
		}
		else { setDynamicProperty(lbl, "valid", false);
			setDynamicProperty(lbl, "invalid", true);
		}
	}
	else
	{
		lbl->setVisible(false);
	}

}

void InfoPage::setConnectionStatus(bool failed)
{
	if(failed)
		setStatusLabel(ui->lblConnectionStatus, tr("Error: Connection failed!"), "red");
	else
		setStatusLabel(ui->lblConnectionStatus, tr("Connected"));
}

void InfoPage::refreshInfoWidget()
{
	if(supportsIdentification())
		setStatusLabel(ui->lblIdentifyStatus);
	else
		setStatusLabel(ui->lblIdentifyStatus, tr("Your hardware revision does not support the identify feature"));

	if(supportsCalibration())
		ui->btnCalibrate->setVisible(true);

	if ( ui->paramLayout != NULL )
	{
		QLayoutItem* item;
		while ( ( item = ui->paramLayout->takeAt( 0 ) ) != NULL )
		{
			delete item->widget();
			delete item;
		}
	}

	int pos = 0;
	for (auto key : m_info_params.keys()) {
		QLabel *valueLbl = new QLabel(this);
		QLabel *keyLbl = new QLabel(this);
		valueLbl->setText(m_info_params.value(key));
		valueLbl->setWordWrap(true);
		keyLbl->setText(key);
		keyLbl->setMinimumWidth(240);
		keyLbl->setMaximumWidth(240);
		ui->paramLayout->addWidget(keyLbl, pos, 0, 1, 1);
		ui->paramLayout->addWidget(valueLbl, pos, 1, 1, 1);
		pos++;
	}

	if (m_advanced) {
		ui->paramLayout->setRowMinimumHeight(pos, 20);
		pos++;
		ui->paramLayout->addWidget(new QLabel("Advanced"), pos, 0, 1, 1);
		pos++;
		for (auto key : m_info_params_advanced.keys()) {
			QLabel *valueLbl = new QLabel(this);
			QLabel *keyLbl = new QLabel(this);

			valueLbl->setWordWrap(true);
			valueLbl->setText(m_info_params_advanced.value(key));
			keyLbl->setText(key);

			ui->paramLayout->addWidget(keyLbl, pos, 0, 1, 1);
			ui->paramLayout->addWidget(valueLbl, pos, 1, 1, 1);
			pos++;
		}
	}
}

QString InfoPage::uri() const
{
	return m_uri;
}

void InfoPage::setUri(QString uri)
{
	m_uri = uri;
}

QPushButton* InfoPage::forgetDeviceButton()
{
	return ui->btnForget;
}

void InfoPage::identifyDevice(bool clicked)
{
	if (clicked) {
		/* If identification is already on for
		 * this device, don't start it again
		 */
		if (m_search_interrupted) {
			return;
		}
		m_search_interrupted = true;
		Q_EMIT stopSearching(true);

		if (!m_connected) {
			m_ctx = iio_create_context_from_uri(m_uri.toStdString().c_str());
		}

		if (!m_ctx) {
			return;
		}
		startIdentification(true);
	} else {
		ledTimeout();
	}
}

void InfoPage::blinkTimeout()
{
}

void InfoPage::startIdentification(bool start)
{
	if (!m_connected) {
		iio_context_destroy(m_ctx);
		m_ctx = nullptr;
	}
	if (m_search_interrupted) {
		m_search_interrupted = false;
		Q_EMIT stopSearching(false);
	}
}

void InfoPage::ledTimeout()
{
	if (m_led_timer->isActive()) {
		m_led_timer->stop();
	} else {
		return;
	}

	if (m_blink_timer->isActive()) {
		m_blink_timer->stop();
	}

	startIdentification(false);

	if (m_ctx) {
		if (!m_connected) {
			iio_context_destroy(m_ctx);
			m_ctx = nullptr;
		}
	}

	if (m_search_interrupted) {
		m_search_interrupted = false;
		Q_EMIT stopSearching(false);
	}
}

QPushButton* InfoPage::identifyDeviceButton()
{
	return ui->btnIdentify;
}

QPushButton* InfoPage::connectButton()
{
	return ui->btnConnect;
}

QPushButton* InfoPage::calibrateButton()
{
	return ui->btnCalibrate;
}


bool InfoPage::supportsIdentification()
{
	QString model = m_info_params["Model"];
	if(identifySupportedModels.contains(model))
		return true;
	return false;
}

bool InfoPage::supportsCalibration()
{
	QString model = m_info_params["Model"];
	if(calibrateSupportedModels.contains(model))
		return true;
	return false;
}

QString InfoPage::getFirmwareVersion() const
{
	return m_info_params.value("Firmware version", "");
}

QString InfoPage::getSerialNumber() const
{
	return m_info_params.value("Serial", "");
}

M2kInfoPage::M2kInfoPage(QString uri,
			 Preferences* prefPanel,
			 PhoneHome* phoneHome,
			 struct iio_context *ctx,
			 QWidget *parent) :
	InfoPage(uri, prefPanel, phoneHome, ctx, parent),
	m_fabric_channel(nullptr), m_m2k(nullptr), m_refreshTemperatureTimer(nullptr)
{
	ui->btnCalibrate->setEnabled(false);
	ui->extraWidget->setFrameShape(QFrame::NoFrame);
	ui->extraWidget->setOpenExternalLinks(true);
	ui->extraWidget->setSource(QUrl("qrc:/m2k.html"));
	ui->extraWidget->setMaximumHeight(700);
	ui->extraWidget->setMinimumHeight(700);
	setConnectionStatusLabel(tr("Not connected"));
	setCalibrationInfoLabel(tr("Always disconnect analog inputs/outputs before calibration"));
	if(ctx) {
		m_m2k = libm2k::context::m2kOpen(ctx, uri.toLocal8Bit().constData());
	}

	m_refreshTemperatureTimer = new QTimer(this);
	connect(m_refreshTemperatureTimer, SIGNAL(timeout()), this, SLOT(refreshTemperature()));
	m_refreshTemperatureTimer->start(m_temperatureUpdateInterval);

}

M2kInfoPage::~M2kInfoPage()
{

}

void M2kInfoPage::getDeviceInfo()
{
	InfoPage::getDeviceInfo();
	refreshTemperature();
}

void M2kInfoPage::refreshTemperature()
{

	libm2k::context::M2k *temp_m2k = nullptr;
	if(!m_ctx) {
		temp_m2k = libm2k::context::m2kOpen(m_uri.toLocal8Bit().constData());
	} else {
		temp_m2k = m_m2k;
	}

	if(temp_m2k) {
		auto dmm = temp_m2k->getDMM("ad9963");
		auto ch = dmm->readChannel("temp0");
		auto val = ch.value;
		m_info_params["Temperature"] = QString::number(val);
	}

	if(!m_ctx) {
		libm2k::context::contextClose(temp_m2k, false);
	}

	refreshInfoWidget();
}

void M2kInfoPage::setCtx(iio_context *ctx)
{
	InfoPage::setCtx(ctx);
	if(ctx) {
		m_m2k = libm2k::context::m2kOpen(ctx, "");
	} else {
		m_m2k = nullptr;
	}
}

void M2kInfoPage::startIdentification(bool start)
{
	if(supportsIdentification()) {
		if (start) {
			struct iio_device *m2k_fabric = iio_context_find_device(m_ctx,
										"m2k-fabric");
			if (!m2k_fabric) {
				setStatusLabel(ui->lblIdentifyStatus, tr("Can't identify this device."), "red");
				if (!m_connected) {
					iio_context_destroy(m_ctx);
					m_ctx = nullptr;
				}
				m_fabric_channel = nullptr;

				if (m_search_interrupted) {
					m_search_interrupted = false;
					Q_EMIT stopSearching(false);
				}
				return;
			}

			m_fabric_channel = iio_device_find_channel(m2k_fabric, "voltage4", true);
			if (m_fabric_channel) {
				m_led_timer->start(3000);
				m_blink_timer->start(100);
			} else {
				setStatusLabel(ui->lblIdentifyStatus, tr("Can't identify device. Please try to update your firmware!"), "red");

				if (!m_connected) {
					iio_context_destroy(m_ctx);
					m_ctx = nullptr;
				}
				m_fabric_channel = nullptr;

				if (m_search_interrupted) {
					m_search_interrupted = false;
					Q_EMIT stopSearching(false);
				}
			}
		} else {
			if (!m_fabric_channel)
				return;
			if (m_ctx) {
				iio_channel_attr_write_bool(m_fabric_channel,
							    "done_led_overwrite_powerdown",
							    false);
			}
			m_fabric_channel = nullptr;
		}
	}
	else
	{
		setStatusLabel(ui->lblIdentifyStatus, tr("Your hardware revision does not support the identify feature"));
	}
}

void M2kInfoPage::blinkTimeout()
{
	if (!m_fabric_channel)
		return;
	bool oldVal;
	iio_channel_attr_read_bool(m_fabric_channel,
				   "done_led_overwrite_powerdown",
				   &oldVal);
	iio_channel_attr_write_bool(m_fabric_channel,
				    "done_led_overwrite_powerdown",
				    !oldVal);
}

int M2kInfoPage::checkLatestFwVersion(const QString &currentVersion) const {
	if (m_phoneHome->getM2kVersion().isEmpty()) {
		return -1;
	}

	if (m_phoneHome->getM2kVersion() == currentVersion) {
		return 1;
	}

	return 0;
}
