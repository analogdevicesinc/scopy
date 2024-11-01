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

#include "regmapstylehelper.hpp"
#include "utils.hpp"

#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <style.h>
#include "dynamicWidget.h"
#include <pluginbase/preferences.h>
#include <qboxlayout.h>
#include <registermapsettingsmenu.hpp>
#include <stylehelper.h>
#include <style_properties.h>

using namespace scopy::regmap;

RegmapStyleHelper *RegmapStyleHelper::pinstance_{nullptr};

RegmapStyleHelper::RegmapStyleHelper(QObject *parent) {}

RegmapStyleHelper *RegmapStyleHelper::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new RegmapStyleHelper(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

void RegmapStyleHelper::regmapSettingsMenu(RegisterMapSettingsMenu *settings, QString objectName)
{
	if(!objectName.isEmpty() && settings)
		settings->setObjectName(objectName);

	Style::setStyle(settings->pathButton, style::properties::button::basicButton);
	Style::setStyle(settings->registerDump, style::properties::button::basicButton);
	Style::setStyle(settings->readInterval, style::properties::button::basicButton);
	Style::setStyle(settings->writeListOfValuesButton, style::properties::button::basicButton);
	Style::setStyle(settings->hexaPrefix1, style::properties::label::menuBig);
	Style::setStyle(settings->hexaPrefix2, style::properties::label::menuBig);
}

void RegmapStyleHelper::toggleSelectedRegister(QWidget *widget, bool toggle)
{
	QString styleSheet = "";
	if(toggle) {
		styleSheet =
			"background-color:" + Style::getAttribute(json::theme::interactive_primary_disabled) + " ;";
	} else {
		styleSheet = "background-color:" + Style::getAttribute(json::theme::background_primary) + " ;";
	}

	widget->setStyleSheet(styleSheet);
}

void RegmapStyleHelper::applyBitfieldValueColorPreferences(BitFieldSimpleWidget *widget)
{
	{
		if(widget->value->text() != "N/R") {
			scopy::Preferences *p = scopy::Preferences::GetInstance();
			QString background = p->get("regmap_color_by_value").toString();

			if(background.contains("Bitfield background")) {
				widget->mainFrame->setStyleSheet("background-color: " +
								 getColorBasedOnValue(widget->value->text()));
			}

			if(background.contains("Bitfield text")) {
				widget->value->setStyleSheet("color: " + getColorBasedOnValue(widget->value->text()));
			}
		}
	}
}

void RegmapStyleHelper::applyRegisterValueColorPreferences(RegisterSimpleWidget *widget)
{

	if(widget->value->text() != "N/R") {
		scopy::Preferences *p = scopy::Preferences::GetInstance();
		QString background = p->get("regmap_color_by_value").toString();

		if(background.contains("Register background")) {
			widget->regBaseInfoWidget->setStyleSheet("background-color: " +
								 getColorBasedOnValue(widget->value->text()));
		}

		if(background.contains("Register text")) {
			widget->value->setStyleSheet("color: " + getColorBasedOnValue(widget->value->text()));
		}
	}
}

void RegmapStyleHelper::BitFieldDetailedWidgetStyle(BitFieldDetailedWidget *widget, QString objectName)
{
	if(!objectName.isEmpty())
		widget->setObjectName(objectName);

	QString style = QString(R"css(
			.scopy--regmap--BitFieldDetailedWidget {
				padding-right : 2px ;
				padding-top : 2px ;
				padding-bottom : 2px ;
				margin-left : 0px ;
			}
                        .QFrame {
                                background-color: &&frameBackground&& ;
                                border-radius: 4px;
                        }
                        QWidget {
				background-color: Transparent ;
						}
						QComboBox  {
						 border-bottom: 0px;
						}
                        )css");

	style.replace("&&frameBackground&&", Style::getAttribute(json::theme::interactive_subtle_disabled));

	widget->setMinimumWidth(10);
	widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	Style::setStyle(widget->nameLabel, style::properties::label::menuSmall);

	if(widget->valueSwitch) {
		StyleHelper::MenuOnOffSwitch(dynamic_cast<SmallOnOffSwitch *>(widget->valueSwitch), "onOffSwitch");
	}
	widget->setFixedHeight(96);

	widget->setStyleSheet(style);
}

void RegmapStyleHelper::BitFieldSimpleWidgetStyle(BitFieldSimpleWidget *widget, QString objectName)
{
	if(!objectName.isEmpty())
		widget->setObjectName(objectName);

	Style::setStyle(widget->value, style::properties::label::menuSmall);
	Style::setStyle(widget->mainFrame, style::properties::regmap::hoverWidget, true, true);
	Style::setStyle(widget->mainFrame, style::properties::regmap::bitfieldSimpleWidget);

	if(widget->value->text() != "N/R") {
		RegmapStyleHelper::applyBitfieldValueColorPreferences(widget);
	}
}

void RegmapStyleHelper::RegisterSimpleWidgetStyle(RegisterSimpleWidget *widget, QString objectName)
{
	if(!objectName.isEmpty())
		widget->setObjectName(objectName);

	QString style = QString(R"css(

			.QFrame {
				background-color: &&frameBackground&& ;
				border-radius: 4px;
				margin-left: 2px;
				margin-right: 2px;
			}


			.scopy--regmap--RegisterSimpleWidget:hover  {
				border: 1px solid &&hoverBackground&& ;
				border-radius: 4px;
			}
			)css");

	style.replace("&&hoverBackground&&", Style::getAttribute(json::theme::content_default));
	style.replace("&&frameBackground&&", Style::getAttribute(json::theme::interactive_subtle_disabled));

	Style::setStyle(widget->regBaseInfoWidget, style::properties::regmap::hoverWidget, true, true);
	Style::setStyle(widget->value, style::properties::label::menuSmall);
	Style::setStyle(widget->registerNameLabel, style::properties::label::menuSmall);

	widget->setStyleSheet(style);

	if(widget->value->text() != "N/R") {
		RegmapStyleHelper::applyRegisterValueColorPreferences(widget);
	}
}

void RegmapStyleHelper::smallBlueButton(QPushButton *button, QString objectName)
{
	if(!objectName.isEmpty())
		button->setObjectName(objectName);

	button->setFixedSize(16, 16);
	button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	Style::setStyle(button, style::properties::button::smallSquareIconButton, true, true);
}

QString RegmapStyleHelper::getColorBasedOnValue(QString value)
{
	uint32_t colorIndex = Utils::convertQStringToUint32(value) % 16;
	return StyleHelper::getChannelColor(colorIndex);
}
