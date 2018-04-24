/*
 * Copyright 2018 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "info_page.hpp"
#include "ui_info_page.h"
#include "preferences.h"

#include <QString>

using namespace std;
using namespace adiscope;

InfoPage::InfoPage(QString uri, Preferences *pref,
                   struct iio_context *ctx,
                   QWidget *parent) :
        QWidget(parent),
        ui(new Ui::InfoPage),
        m_uri(uri),
        m_ctx(ctx),
        m_advanced(false),
        prefPanel(pref)
{
        ui->setupUi(this);
        ui->paramLabel->setText(uri);

        if (uri.startsWith("usb:")) {
                ui->btnForget->setEnabled(false);
        }
        connect(prefPanel, &Preferences::notify, this, &InfoPage::readPreferences);
        readPreferences();
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
		char *ctx_git_tag;
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
        }
        return QPair<bool, QString>(advanced, key);
}

void InfoPage::setConnectionStatus(bool failed)
{
        (failed) ? ui->lblConnectionStatus->setText("Error: Connection failed!") :
                   ui->lblConnectionStatus->setText("");
}

void InfoPage::refreshInfoWidget()
{
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
                valueLbl->setStyleSheet("color: white");
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
                        valueLbl->setText(m_info_params_advanced.value(key));
                        valueLbl->setStyleSheet("color: white");
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

QPushButton* InfoPage::identifyDeviceButton()
{
        return ui->btnIdentify;
}

QPushButton* InfoPage::connectButton()
{
        return ui->btnConnect;
}
