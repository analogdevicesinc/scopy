/*
 * Copyright (c) 2023 Analog Devices Inc.
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

#ifndef SWIOTADVMENU_H
#define SWIOTADVMENU_H

#include <QWidget>
#include <iio-widgets/iiowidget.h>
#include <gui/spinbox_a.hpp>
#include <iioutil/connection.h>

#define OUTPUT_CHNL "output"
#define INPUT_CHNL "input"

namespace scopy::swiotrefactor {

class BufferMenu : public QWidget
{
	Q_OBJECT
public:
	explicit BufferMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
			    QMap<QString, iio_channel *> chnls = {});
	~BufferMenu();

	virtual QString getInfoMessage();

	QList<QWidget *> getWidgetsList();
	void addMenuWidget(QWidget *widget);

	void setOffsetScalePair(const std::pair<double, double> &newOffsetScalePair);
	double convertFromRaw(double rawValue);

public Q_SLOTS:
	virtual void onBroadcastThreshold();
	virtual void onDiagSamplingChange(QString samplingFreq);
	virtual void onSamplingFreqWrite(QString data, QString dataOptions);
	virtual void onRunBtnsPressed(bool en);

Q_SIGNALS:
	void diagnosticFunctionUpdated();
	void diagSamplingFreqChange(QString data);
	void samplingFrequencyUpdated(int sr);
	void freqChangeStart();
	void freqChangeEnd();
	void thresholdChangeStart();
	void thresholdChangeEnd();

protected:
	IIOWidget *m_samplingFreq;
	QString m_chnlFunction;
	Connection *m_connection;
	QMap<QString, iio_channel *> m_chnls;
	std::pair<double, double> m_offsetScalePair = {0, 1};

private:
	QList<QWidget *> m_widgetsList;
};

class CurrentInLoopMenu : public BufferMenu
{
	Q_OBJECT
public:
	explicit CurrentInLoopMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
				   QMap<QString, iio_channel *> chnls = {});
	~CurrentInLoopMenu();

	QString getInfoMessage() override;

private Q_SLOTS:
	void updateCnvtLabel(QString data);

private:
	QLabel *m_cnvtLabel;
};

class DigitalInLoopMenu : public BufferMenu
{
	Q_OBJECT
public:
	explicit DigitalInLoopMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
				   QMap<QString, iio_channel *> chnls = {});
	~DigitalInLoopMenu();

	QString getInfoMessage() override;
public Q_SLOTS:
	void onBroadcastThreshold() override;
	void onRunBtnsPressed(bool en) override;
	void onEmitStatus(int retCode);

private Q_SLOTS:
	void updateCnvtLabel(QString data);

private:
	IIOWidget *m_threshold;
	QLabel *m_cnvtLabel;
};

class VoltageOutMenu : public BufferMenu
{
	Q_OBJECT
public:
	explicit VoltageOutMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
				QMap<QString, iio_channel *> chnls = {});
	~VoltageOutMenu();

	QString getInfoMessage() override;

private Q_SLOTS:
	void updateCnvtLabel(QString data);

private:
	QLabel *m_cnvtLabel;
};

class CurrentOutMenu : public BufferMenu
{
	Q_OBJECT
public:
	explicit CurrentOutMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
				QMap<QString, iio_channel *> chnls = {});
	~CurrentOutMenu();

	QString getInfoMessage() override;

private Q_SLOTS:
	void updateCnvtLabel(QString data);

private:
	QLabel *m_cnvtLabel;
};

class DiagnosticMenu : public BufferMenu
{
	Q_OBJECT
public:
	explicit DiagnosticMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
				QMap<QString, iio_channel *> chnls = {});
	~DiagnosticMenu();

public Q_SLOTS:
	void onDiagSamplingChange(QString samplingFreq) override;
	void onSamplingFreqWrite(QString data, QString dataOptions) override;
};

class DigitalInMenu : public BufferMenu
{
	Q_OBJECT
public:
	explicit DigitalInMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
			       QMap<QString, iio_channel *> chnls = {});
	~DigitalInMenu();

public Q_SLOTS:
	void onBroadcastThreshold() override;
	void onRunBtnsPressed(bool en) override;
	void onEmitStatus(int retCode);

private:
	IIOWidget *m_threshold;
};

class WithoutAdvSettings : public BufferMenu
{
	Q_OBJECT
public:
	explicit WithoutAdvSettings(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
				    QMap<QString, iio_channel *> chnls = {});
	~WithoutAdvSettings();
};

class BufferMenuBuilder
{

public:
	enum MenuType
	{
		CURRENT_IN_LOOP = 0,
		DIGITAL_IN = 1,
		DIGITAL_IN_LOOP = 2,
		VOLTAGE_OUT = 3,
		CURRENT_OUT = 4,
		VOLTAGE_IN = 5,
		CURRENT_IN_EXT = 6,
		CURRENT_IN_LOOP_HART = 7,
		CURRENT_IN_EXT_HART = 8,
		RESISTANCE = 9,
		DIAGNOSTIC = 10,
		HIGH_Z = 11
	};

	static int decodeFunctionName(QString function)
	{
		if(function.compare("voltage_in") == 0) {
			return VOLTAGE_IN;
		} else if(function.compare("current_in_ext") == 0) {
			return CURRENT_IN_EXT;
		} else if(function.compare("current_in_loop_hart") == 0) {
			return CURRENT_IN_LOOP_HART;
		} else if(function.compare("current_in_ext_hart") == 0) {
			return CURRENT_IN_EXT_HART;
		} else if(function.compare("resistance") == 0) {
			return RESISTANCE;
		} else if(function.compare("current_in_loop") == 0) {
			return CURRENT_IN_LOOP;
		} else if(function.compare("digital_input") == 0) {
			return DIGITAL_IN;
		} else if(function.compare("digital_input_loop") == 0) {
			return DIGITAL_IN_LOOP;
		} else if(function.compare("voltage_out") == 0) {
			return VOLTAGE_OUT;
		} else if(function.compare("current_out") == 0) {
			return CURRENT_OUT;
		} else if(function.compare("diagnostic") == 0) {
			return DIAGNOSTIC;
		} else if(function.compare("high_z") == 0) {
			return HIGH_Z;
		} else {
			return -1;
		}
	}

	static BufferMenu *newAdvMenu(QWidget *widget, QString function, Connection *conn,
				      QMap<QString, iio_channel *> chnls)
	{
		int menu_type = decodeFunctionName(function);
		switch(menu_type) {
		case CURRENT_IN_LOOP:
			return new CurrentInLoopMenu(widget, function, conn, chnls);
		case CURRENT_IN_LOOP_HART:
			return new CurrentInLoopMenu(widget, function, conn, chnls);
		case DIGITAL_IN_LOOP:
			return new DigitalInLoopMenu(widget, function, conn, chnls);
		case VOLTAGE_OUT:
			return new VoltageOutMenu(widget, function, conn, chnls);
		case CURRENT_OUT:
			return new CurrentOutMenu(widget, function, conn, chnls);
		case DIGITAL_IN:
			return new DigitalInMenu(widget, function, conn, chnls);
		case DIAGNOSTIC:
			return new DiagnosticMenu(widget, function, conn, chnls);
		default:
			return new WithoutAdvSettings(widget, function, conn, chnls);
		}
	}
};

} // namespace scopy::swiotrefactor

#endif // SWIOTADVMENU_H
