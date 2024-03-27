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

#include "qboxlayout.h"
#include "qcombobox.h"
#include "qlabel.h"

#include <QWidget>

#include <gui/spinbox_a.hpp>
#include <iioutil/connection.h>
#include <string>

namespace scopy {
class CustomSwitch;

namespace swiotrefactor {

#define SLEW_DISABLE_IDX 0
#define OUTPUT_CHNL "output"
#define INPUT_CHNL "input"

class BufferMenu : public QWidget
{
	Q_OBJECT
public:
	explicit BufferMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
			    iio_channel *chnl = nullptr);
	~BufferMenu();

	virtual void init();
	virtual QString getInfoMessage();
	QVector<QBoxLayout *> getMenuLayers();
	QMap<QString, QMap<QString, QStringList>> getAttrValues();

	void addMenuLayout(QBoxLayout *layout);
	void setupVerticalSettingsMenu(QWidget *settingsWidget, QString unit, double yMin, double yMax);
	void setAttrValues(QMap<QString, QMap<QString, QStringList>> values);
	double convertFromRaw(int rawValue, QString chnlType = OUTPUT_CHNL);
	void connectSignalsToSlots();
public Q_SLOTS:
	void onAttrWritten(QMap<QString, QMap<QString, QStringList>> values);
	void onSamplingFreqChanged(int idx);
Q_SIGNALS:
	void attrValuesChanged(QString attrName, QString chnlType);
	void mapUpdated();
	void thresholdControlEnable(bool enable);
	void broadcastThresholdReadForward(QString value);
	void broadcastThresholdReadBackward(QString value);
	void diagnosticFunctionUpdated();
	void samplingFrequencyUpdated(int sr);
	void setUnitPerDivision(double val);
	void unitPerDivisionChanged(double val);

protected:
	void setAvailableOptions(QComboBox *list, QString attrName);

protected:
	QWidget *m_widget;
	QString m_chnlFunction;
	Connection *m_connection;
	iio_channel *m_chnl;
	QMap<QString, QMap<QString, QStringList>> m_attrValues;
	PositionSpinButton *m_unitPerDivision;
	QComboBox *m_samplingFreqOptions;

private:
	QVector<QBoxLayout *> m_menuLayers;
};

class CurrentInLoopMenu : public BufferMenu
{
	Q_OBJECT
public:
	explicit CurrentInLoopMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
				   iio_channel *chnl = nullptr);
	~CurrentInLoopMenu();
	void init();
	void connectSignalsToSlots();
	QString getInfoMessage();
public Q_SLOTS:
	void dacCodeChanged(double value);
	void onMapUpdated();

private:
	PositionSpinButton *m_dacCodeSpinButton;
	QLabel *m_dacLabel;
};

class DigitalInLoopMenu : public BufferMenu
{
	Q_OBJECT
public:
	explicit DigitalInLoopMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
				   iio_channel *chnl = nullptr);
	~DigitalInLoopMenu();
	void init();
	void connectSignalsToSlots();
	QString getInfoMessage();
public Q_SLOTS:
	void dacCodeChanged(double value);
	void thresholdChanged();
	void onMapUpdated();
	void onBroadcastThresholdRead(QString value);
	void onThresholdControlEnable(bool enabled);

private:
	QLabel *m_titleLabel;
	QLineEdit *m_thresholdLineEdit;
	PositionSpinButton *m_dacCodeSpinButton;
	QLabel *m_dacLabel;
};

class VoltageOutMenu : public BufferMenu
{
	Q_OBJECT
public:
	explicit VoltageOutMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
				iio_channel *chnl = nullptr);
	~VoltageOutMenu();
	void init();
	void connectSignalsToSlots();
	QString getInfoMessage();
public Q_SLOTS:
	void slewStepIndexChanged(int idx);
	void slewRateIndexChanged(int idx);
	void slewIndexChanged(int idx);
	void dacCodeChanged(double value);
	void onMapUpdated();

private:
	PositionSpinButton *m_dacCodeSpinButton;
	QLabel *m_dacLabel;
	QComboBox *m_slewOptions;
	QComboBox *m_slewStepOptions;
	QComboBox *m_slewRateOptions;

	void setAvailableOptions(QComboBox *list, QString attrName);
};

class CurrentOutMenu : public BufferMenu
{
	Q_OBJECT
public:
	explicit CurrentOutMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
				iio_channel *chnl = nullptr);
	~CurrentOutMenu();
	void init();
	void connectSignalsToSlots();
	QString getInfoMessage();

public Q_SLOTS:
	void slewStepIndexChanged(int idx);
	void slewRateIndexChanged(int idx);
	void slewIndexChanged(int idx);
	void dacCodeChanged(double value);
	void onMapUpdated();

private:
	PositionSpinButton *m_dacCodeSpinButton;
	QComboBox *m_slewOptions;
	QComboBox *m_slewStepOptions;
	QComboBox *m_slewRateOptions;
	QLabel *m_dacLabel;

	void setAvailableOptions(QComboBox *list, QString attrName);
};

class DiagnosticMenu : public BufferMenu
{
	Q_OBJECT
public:
	explicit DiagnosticMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
				iio_channel *chnl = nullptr);
	~DiagnosticMenu();
	void init();
	void connectSignalsToSlots();
public Q_SLOTS:
	void diagIndexChanged(int idx);
	void onMapUpdated();

private:
	QComboBox *m_diagOptions;

	void setAvailableOptions(QComboBox *list, QString attrName);
};

class DigitalInMenu : public BufferMenu
{
	Q_OBJECT
public:
	explicit DigitalInMenu(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
			       iio_channel *chnl = nullptr);
	~DigitalInMenu();
	void init();
	void connectSignalsToSlots();
public Q_SLOTS:
	void thresholdChanged();
	void onMapUpdated();
	void onBroadcastThresholdRead(QString value);
	void onThresholdControlEnable(bool enabled);

private:
	QLabel *m_titleLabel;
	QLineEdit *m_thresholdLineEdit;
};

class WithoutAdvSettings : public BufferMenu
{
	Q_OBJECT
public:
	explicit WithoutAdvSettings(QWidget *parent = nullptr, QString chnlFunction = "", Connection *conn = nullptr,
				    iio_channel *chnl = nullptr);
	~WithoutAdvSettings();
	void init();
	void connectSignalsToSlots();
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

	static BufferMenu *newAdvMenu(QWidget *widget, QString function, Connection *conn, iio_channel *chnl)
	{
		int menu_type = decodeFunctionName(function);
		switch(menu_type) {
		case CURRENT_IN_LOOP:
			return new CurrentInLoopMenu(widget, function, conn, chnl);
		case CURRENT_IN_LOOP_HART:
			return new CurrentInLoopMenu(widget, function, conn, chnl);
		case DIGITAL_IN_LOOP:
			return new DigitalInLoopMenu(widget, function, conn, chnl);
		case VOLTAGE_OUT:
			return new VoltageOutMenu(widget, function, conn, chnl);
		case CURRENT_OUT:
			return new CurrentOutMenu(widget, function, conn, chnl);
		case DIGITAL_IN:
			return new DigitalInMenu(widget, function, conn, chnl);
		case DIAGNOSTIC:
			return new DiagnosticMenu(widget, function, conn, chnl);
		default:
			return new WithoutAdvSettings(widget, function, conn, chnl);
		}
	}
};

} // namespace swiotrefactor
} // namespace scopy

#endif // SWIOTADVMENU_H
