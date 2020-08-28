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


#ifndef INFO_PAGE_HPP
#define INFO_PAGE_HPP

#include <QWidget>
#include <QMap>
#include <QPushButton>
#include <QFuture>
#include <QLabel>

#include "iio.h"

namespace Ui {
class InfoPage;
}

namespace adiscope {
class Preferences;
class Calibration;
class InfoPage : public QWidget
{
	Q_OBJECT

public:
	explicit InfoPage(QString uri,
			  Preferences* prefPanel,
			  struct iio_context *ctx = nullptr,
			  QWidget *parent = 0);
	virtual ~InfoPage();


	struct iio_context *ctx() const;
	void setCtx(iio_context *ctx);

	QString uri() const;
	void setUri(QString uri);

	QPushButton *forgetDeviceButton();
	QPushButton *identifyDeviceButton();
	QPushButton *connectButton();
	QPushButton *calibrateButton();

	virtual void getDeviceInfo();
	void refreshInfoWidget();

	void setConnectionStatus(bool);
	bool supportsIdentification();
	bool supportsCalibration();

public Q_SLOTS:
	void readPreferences();
	void identifyDevice(bool clicked = true);
	void setStatusLabel(QLabel *lbl, QString str = "", QString color="white");


	void setCalibrationStatusLabel(QString str = "", QString color = "white");
	void setCalibrationInfoLabel(QString str = "", QString color = "white");
	void setIdentyifyLabel(QString str = "", QString color = "white");
	void setConnectionStatusLabel(QString str = "", QString color = "white");

private Q_SLOTS:
	virtual void blinkTimeout();
	void ledTimeout();

Q_SIGNALS:
	void stopSearching(bool);

protected:
	virtual void startIdentification(bool);

private:
	QPair<bool, QString> translateInfoParams(QString);
	const QStringList identifySupportedModels = {"Analog Devices M2k Rev.C (Z7010)","Analog Devices M2k Rev.D (Z7010)"};
	const QStringList calibrateSupportedModels = {"Analog Devices M2k Rev.C (Z7010)","Analog Devices M2k Rev.D (Z7010)"};

protected:
	Ui::InfoPage *ui;
	QString m_uri;
	struct iio_context *m_ctx;
	bool m_advanced;
	Preferences *prefPanel;
	QMap<QString, QString> m_info_params;
	QMap<QString, QString> m_info_params_advanced;
	QTimer *m_led_timer;
	QTimer *m_blink_timer;
	bool m_connected;
	bool m_search_interrupted;
};


class M2kInfoPage : public InfoPage
{
	Q_OBJECT
public:
	explicit M2kInfoPage(QString uri,
			     Preferences* prefPanel,
			     struct iio_context *ctx = nullptr,
			     QWidget *parent = 0);
	~M2kInfoPage();
	void getDeviceInfo();

protected:
	virtual void startIdentification(bool);

private Q_SLOTS:
	void blinkTimeout();

private:
	struct iio_channel *m_fabric_channel;
	QFuture<void> calibration_thread;
};


class InfoPageBuilder
{
public:
	enum InfoPageType {
		GENERIC = 0,
		M2K = 1,
	};

	static InfoPage* newPage(InfoPageType page_type,
				 QString uri,
				 Preferences* prefPanel,
				 struct iio_context *ctx = nullptr,
				 QWidget *parent = 0)
	{
		switch (page_type) {
		case GENERIC: return new InfoPage(uri, prefPanel,
						  ctx, parent);
		case M2K: return new M2kInfoPage(uri, prefPanel,
						 ctx, parent);
		}
		return nullptr;
	}
};
}
#endif // INFO_PAGE_HPP
