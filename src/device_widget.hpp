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

#ifndef DEVICE_WIDGET_HPP
#define DEVICE_WIDGET_HPP

#include "iio.h"

#include <QPushButton>
#include <QWidget>

namespace Ui {
class Device;
}

namespace adiscope {
class InfoPage;
class ToolLauncher;

class DeviceWidget : public QWidget {
	Q_OBJECT
public:
	explicit DeviceWidget(QString uri, QString name,
			      ToolLauncher *parent = nullptr);
	virtual ~DeviceWidget();

	QPushButton *deviceButton() const;
	QPushButton *connectButton() const;
	QPushButton *calibrateButton() const;

	void setChecked(bool);
	bool isChecked();

	QString uri() const;
	void setUri(const QString &uri);

	void highlightDevice();
	void click();

	InfoPage *infoPage() const;
	void setInfoPage(InfoPage *infoPage);

	bool connected() const;
	void setConnected(bool, bool, struct iio_context *ctx = nullptr);

	void setName(QString);

Q_SIGNALS:
	void selected(bool);
	void forgetDevice(QString);
	void identifyDevice(QString);
	void calibrateDevice();

public Q_SLOTS:
	void on_btn_toggled(bool);
	void forgetDevice_clicked(bool);
	void identifyDevice_clicked(bool);

protected:
	Ui::Device *m_ui;
	QString m_uri;
	InfoPage *m_infoPage;
	bool m_connected;
	bool m_selected;
};

class M2kDeviceWidget : public DeviceWidget {
	Q_OBJECT
public:
	explicit M2kDeviceWidget(QString uri, QString name,
				 ToolLauncher *parent = nullptr);
	~M2kDeviceWidget();
};

class DeviceBuilder {
public:
	enum DeviceType {
		GENERIC = 0,
		M2K = 1,
	};

	static DeviceWidget *newDevice(DeviceType dev_type, QString uri,
				       QString name,
				       ToolLauncher *parent = nullptr) {
		switch (dev_type) {
		case GENERIC:
			return new DeviceWidget(uri, name, parent);
		case M2K:
			return new M2kDeviceWidget(uri, name, parent);
		}
		return nullptr;
	}
};

} /* namespace adiscope */

#endif // DEVICE_WIDGET_HPP
