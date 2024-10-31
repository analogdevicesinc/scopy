/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef REGMAPSTYLEHELPER_HPP
#define REGMAPSTYLEHELPER_HPP

#include "titlespinbox.hpp"
#include "searchbarwidget.hpp"

#include <QCheckBox>
#include <QLabel>
#include <QMap>
#include <QObject>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>

#include <register/bitfield/bitfielddetailedwidget.hpp>
#include <register/registersimplewidget.hpp>

#include <register/bitfield/bitfielddetailedwidget.hpp>
#include <register/bitfield/bitfieldsimplewidget.hpp>

#include <register/registersimplewidget.hpp>

namespace scopy::regmap {

class RegisterMapSettingsMenu;

class RegmapStyleHelper
{
protected:
	RegmapStyleHelper(QObject *parent = nullptr);
	~RegmapStyleHelper();

public:
	// singleton
	RegmapStyleHelper(RegmapStyleHelper &other) = delete;
	void operator=(const RegmapStyleHelper &) = delete;
	static RegmapStyleHelper *GetInstance();

	static void initColorMap();
	static QString getColor(QString id);

	static void DeviceRegisterMap(QWidget *widget);
	static QString PartialFrameWidgetStyle();
	static QString FrameWidgetStyle();
	static QString bigTextLabelStyle();
	static void bigTextLabel(QLabel *label, QString objectName = "");
	static void labelStyle(QLabel *label, QString objectName);

	static void regmapSettingsMenu(RegisterMapSettingsMenu *settings, QString objectName = "");
	static void grayBackgroundHoverWidget(QWidget *widget, QString objectName = "");
	static QString BlueButtonStyle();
	static void BlueButton(QPushButton *btn, QString objectName = "");
	static QString checkBoxStyle();
	static void checkBox(QCheckBox *checkbox, QString objectName = "");
	static void BitFieldDetailedWidgetStyle(BitFieldDetailedWidget *widget, QString objectName = "");
	static void BitFieldSimpleWidgetStyle(BitFieldSimpleWidget *widget, QString objectName = "");
	static void RegisterSimpleWidgetStyle(RegisterSimpleWidget *widget, QString objectName = "");
	static QString grayLabelStyle();
	static QString whiteSmallTextLableStyle();
	static QString simpleWidgetStyle();
	static void comboboxStyle(QComboBox *combobox, QString objectName = "");
	static void titleSpinBoxStyle(TitleSpinBox *spinbox, QString objectName = "");
	static void searchBarStyle(SearchBarWidget *searchBar, QString objectName = "");
	static void smallBlueButton(QPushButton *button, QString objectName = "");
	static QString regmapControllerStyle(QWidget *widget, QString objectName = "");
	static QString widgetidthRoundCornersStyle(QWidget *widget, QString objectName = "");
	static QString sliderStyle();
	static QString getColorBasedOnValue(QString value);

private:
	QMap<QString, QString> colorMap;
	static RegmapStyleHelper *pinstance_;
};
} // namespace scopy::regmap
#endif // REGMAPSTYLEHELPER_HPP
