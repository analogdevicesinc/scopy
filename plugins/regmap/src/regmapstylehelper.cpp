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

using namespace scopy::regmap;

RegmapStyleHelper *RegmapStyleHelper::pinstance_{nullptr};

RegmapStyleHelper::RegmapStyleHelper(QObject *parent) {}

RegmapStyleHelper *RegmapStyleHelper::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new RegmapStyleHelper(QApplication::instance()); // singleton has the app as parent
		RegmapStyleHelper::initColorMap();
	}
	return pinstance_;
}

void RegmapStyleHelper::initColorMap()
{
	auto sh = RegmapStyleHelper::GetInstance();
	sh->colorMap.insert("ScopyBlue", Style::getAttribute(json::theme::highlight_color));
	sh->colorMap.insert("UIElementBackground", Style::getAttribute(json::theme::background_1));
	sh->colorMap.insert("LabelText", Style::getAttribute(json::theme::highlight_1));
	sh->colorMap.insert("LabelText2", "rgba(255, 255, 255, 150)");
	sh->colorMap.insert("ScopyBackground", Style::getAttribute(json::theme::background_color));
	sh->colorMap.insert("WidgetBackground", Style::getAttribute(json::theme::background_1));
	sh->colorMap.insert("ButtonPressed", Style::getAttribute(json::theme::highlight_1));
	sh->colorMap.insert("ButtonHover", Style::getAttribute(json::theme::highlight_color));
	sh->colorMap.insert("ButtonDisabled", "grey");
	sh->colorMap.insert("Red", "red");
	sh->colorMap.insert("Transparent", "transparent");
	sh->colorMap.insert("CheckBoxTextColor", "rgba(255, 255, 255, 153)");
	sh->colorMap.insert("CheckBoxIndicatorBorderColor", "rgb(74,100,255)");
	sh->colorMap.insert("ScopyOrange", "#FF7200");
	sh->colorMap.insert("DisabledWidget", "#555555");
}

QString RegmapStyleHelper::getColor(QString id)
{
	auto sh = RegmapStyleHelper::GetInstance();
	return sh->colorMap[id];
}

QString RegmapStyleHelper::PartialFrameWidgetStyle()
{
	QString style = QString(R"css(
                        .QWidget {
                                 border-top: 2px solid black;
                                 border-right: 2px solid black;
                                 border-left: 2px solid black;
                        }
                        )css");

	return style;
}

QString RegmapStyleHelper::FrameWidgetStyle()
{
	QString style = QString(R"css(
                        .QWidget {
                                 border-top: 2px solid black;
                                 border-right: 2px solid black;
                                 border-left: 2px solid black;
                                 border-bottom: 2px solid black;
                                 background-color: &&background_1&& ;
                        }

                        QWidget  {
                            background-color: &&background_1&& ;
                        }

                        )css");
	style.replace("&&background_1&&", RegmapStyleHelper::getColor("WidgetBackground"));

	return style;
}

void RegmapStyleHelper::DeviceRegisterMap(QWidget *widget)
{
	QString style = QString(R"css(
                        .QWidget {
                           background-color: &&widgetBackground&& ;
                        }
                        )css");
	style.replace("&&widgetBackground&&", RegmapStyleHelper::getColor("ScopyBackground"));
	widget->setStyleSheet(style);
}

QString RegmapStyleHelper::bigTextLabelStyle()
{
	QString style = QString(R"css(
                        QLabel {
                        font: normal;
                        font-size: 16px;
                        color: &&labelColor&& ;
                        }
                        )css");
	style.replace("&&labelColor&&", RegmapStyleHelper::getColor("LabelText"));

	return style;
}

void RegmapStyleHelper::bigTextLabel(QLabel *label, QString objectName)
{
	if(!objectName.isEmpty())
		label->setObjectName(objectName);

	label->setStyleSheet(RegmapStyleHelper::bigTextLabelStyle());
}

QString RegmapStyleHelper::BlueButtonStyle()
{
	QString style = QString(R"css(
                        QPushButton {
                             background-color: &&buttonBackground&& ;
                             border-radius: 4px;
                             font-size: 14px;
                             color: white;
                             height: 30px;
                             min-width: 150px;

                        }
                        QPushButton:pressed{ background-color: &&pressedColor&& ; }
                        QPushButton:hover{ background-color: &&hoverColor&& ; }
                        QPushButton:disabled { background-color: &&diabledColor&& ; }

                        )css");

	style.replace("&&buttonBackground&&", RegmapStyleHelper::getColor("ScopyBlue"));
	style.replace("&&pressedColor&&", RegmapStyleHelper::getColor("ButtonPressed"));
	style.replace("&&hoverColor&&", RegmapStyleHelper::getColor("ButtonHover"));
	style.replace("&&diabledColor&&", RegmapStyleHelper::getColor("ButtonDisabled"));

	return style;
}

void RegmapStyleHelper::BlueButton(QPushButton *btn, QString objectName)
{
	if(!objectName.isEmpty())
		btn->setObjectName(objectName);

	btn->setStyleSheet(RegmapStyleHelper::BlueButtonStyle());
}

QString RegmapStyleHelper::checkBoxStyle()
{
	QString style = QString(R"css(
						 QCheckBox {
						 spacing: 8px;
							background-color: &&checkBoxBackgroundColor&& ;
							font-size: 14px;
							font-weight: bold;
							color: &&checkBoxColor&&
						 ;
						 }
						 QCheckBox::indicator {
							 width: 14px;
							 height: 14px;
							 border: 2px solid &&checkBoxIndicatorBorderColor&& ;
							 border-radius: 4px;
						 }
						 QCheckBox::indicator:unchecked { background-color: &&checkBoxIndicatorUnchecked&&; }
						 QCheckBox::indicator:checked { background-color: rgb(74,100,255); }
						)css");

	style.replace("&&checkBoxIndicatorUnchecked&&", RegmapStyleHelper::getColor("Transparent"));
	style.replace("&&checkBoxBackgroundColor&&", RegmapStyleHelper::getColor("Transparent"));
	style.replace("&&checkBoxColor&&", RegmapStyleHelper::getColor("CheckBoxTextColor"));
	style.replace("&&checkBoxIndicatorBorderColor&&", RegmapStyleHelper::getColor("CheckBoxIndicatorBorderColor"));

	return style;
}

void RegmapStyleHelper::checkBox(QCheckBox *checkbox, QString objectName)
{
	if(!objectName.isEmpty())
		checkbox->setObjectName(objectName);

	checkbox->setStyleSheet(RegmapStyleHelper::checkBoxStyle());
}

void RegmapStyleHelper::labelStyle(QLabel *label, QString objectName)
{
	if(!objectName.isEmpty())
		label->setObjectName(objectName);

	QString style = QString(R"css(
                        QLabel {
						 font-size: 12px;
                         font-style: normal;
                         font-weight: normal;
                         text-align: left;
                         color: &&labelColor&& ;
                        }
                        )css");
	style.replace("&&labelColor&&", RegmapStyleHelper::getColor("LabelText2"));
	label->setStyleSheet(style);
}

void RegmapStyleHelper::regmapSettingsMenu(RegisterMapSettingsMenu *settings, QString objectName)
{
	if(!objectName.isEmpty() && settings)
		settings->setObjectName(objectName);

	QString style = QString(R"css(
                        QWidget {
                                 background-color: &&background_1&& ;
                        }
                        QWidget {
                                 font: normal;
                                 color: &&textColor&& ;
                                 font-size: 16px;
                        }
                        )css");

	style.replace("&&background_1&&", RegmapStyleHelper::getColor("Transparent"));
	style.replace("&&textColor&&", RegmapStyleHelper::getColor("LabelText"));

	style += RegmapStyleHelper::BlueButtonStyle();

	RegmapStyleHelper::checkBox(settings->autoread);
	RegmapStyleHelper::bigTextLabel(settings->hexaPrefix1);
	RegmapStyleHelper::bigTextLabel(settings->hexaPrefix2);

	settings->setStyleSheet(style);
}

void RegmapStyleHelper::grayBackgroundHoverWidget(QWidget *widget, QString objectName)
{
	if(!objectName.isEmpty() && widget)
		widget->setObjectName(objectName);

	QString style = QString(R"css(

                        .QFrame {
                                background-color: &&frameBackground&& ;
                                border-radius: 4px;
                                margin-left: 2px;
                                margin-right: 2px;
                                height: 60px;

                        }
                        QWidget {
                                background-color: &&childWidgetBackground&& ;
                                height: 60px;
                        }
                        *[is_selected=true] {
                         background-color: &&selectedBackground&& ;
                        }
                        ::hover {
                            background-color: &&hoverBackground&& ;
                        }

                        )css");

	style.replace("&&frameBackground&&", RegmapStyleHelper::getColor("WidgetBackground"));
	style.replace("&&childWidgetBackground&&", RegmapStyleHelper::getColor("Transparent"));
	style.replace("&&hoverBackground&&", RegmapStyleHelper::getColor("ButtonHover"));
	style.replace("&&selectedBackground&&", RegmapStyleHelper::getColor("ScopyBlue"));

	widget->setStyleSheet(style);
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
                                background-color: &&childWidgetBackground&& ;
						}
						QComboBox  {
						 border-bottom: 0px;
						}
                        )css");

	style.replace("&&frameBackground&&", RegmapStyleHelper::getColor("WidgetBackground"));
	style.replace("&&childWidgetBackground&&", RegmapStyleHelper::getColor("Transparent"));

	widget->setMinimumWidth(10);
	widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	widget->nameLabel->setStyleSheet(RegmapStyleHelper::whiteSmallTextLableStyle());

	if(widget->valueSwitch) {
		StyleHelper::MenuOnOffSwitchButton(dynamic_cast<SmallOnOffSwitch *>(widget->valueSwitch),
						   "onOffSwitch");
	}
	widget->setFixedHeight(96);

	widget->setStyleSheet(style);
}

void RegmapStyleHelper::BitFieldSimpleWidgetStyle(BitFieldSimpleWidget *widget, QString objectName)
{
	if(!objectName.isEmpty())
		widget->setObjectName(objectName);

	QString style = QString(R"css(
						QWidget {
								background-color: &&widgetBackground&& ;
						}
						*[is_selected=true] {
						 background-color: &&selectedBackground&& ;
						}
						)css");
	style.replace("&&widgetBackground&&", RegmapStyleHelper::getColor("Transparent"));
	style.replace("&&selectedBackground&&", RegmapStyleHelper::getColor("ScopyBlue"));
	widget->value->setStyleSheet(RegmapStyleHelper::grayLabelStyle());
	grayBackgroundHoverWidget(widget->mainFrame);

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

	widget->setStyleSheet(style);
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
                        QWidget {
                                background-color: &&childWidgetBackground&& ;
                        }
                        *[is_selected=true] {
                         background-color: &&selectedBackground&& ;
                        }

						.scopy--regmap--RegisterSimpleWidget:hover  {
							border: 1px solid &&hoverBackground&& ;
							border-radius: 4px;
						}
						)css");

	style.replace("&&hoverBackground&&", RegmapStyleHelper::getColor("LabelText"));
	style.replace("&&frameBackground&&", RegmapStyleHelper::getColor("WidgetBackground"));
	style.replace("&&childWidgetBackground&&", RegmapStyleHelper::getColor("Transparent"));
	style.replace("&&selectedBackground&&", RegmapStyleHelper::getColor("ScopyBlue"));

	grayBackgroundHoverWidget(widget->regBaseInfoWidget);
	widget->registerNameLabel->setStyleSheet(RegmapStyleHelper::whiteSmallTextLableStyle());
	widget->value->setStyleSheet(RegmapStyleHelper::grayLabelStyle());

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
	widget->setStyleSheet(style);
}

QString RegmapStyleHelper::grayLabelStyle()
{
	QString style = QString(R"css(
                        QLabel {
                         font-size: 12px;
                         font-style: normal;
                         font-weight: normal;
                         text-align: left;
                         color: &&labelColor&& ;
                        }
                        )css");
	style.replace("&&labelColor&&", RegmapStyleHelper::getColor("LabelText2"));

	return style;
}

QString RegmapStyleHelper::whiteSmallTextLableStyle()
{
	QString style = QString(R"css(
                        QLabel {
                        font: normal;
                        font-size: 12px;
                        color: &&labelColor&& ;
                        }
                        )css");
	style.replace("&&labelColor&&", RegmapStyleHelper::getColor("LabelText"));

	return style;
}

QString RegmapStyleHelper::simpleWidgetStyle()
{
	QString style = QString(R"css(
                        QWidget  {
                            background-color: &&widgetBackground&& ;
							border-radius: 4px;
                        }
                        )css");
	style.replace("&&widgetBackground&&", RegmapStyleHelper::getColor("WidgetBackground"));

	return style;
}

void RegmapStyleHelper::comboboxStyle(QComboBox *combobox, QString objectName)
{
	if(!objectName.isEmpty() && combobox)
		combobox->setObjectName(objectName);

	QString style = QString(R"css(
						QComboBox  {
						 color: &&textColor&& ;
						 font-size: 12px;
						 padding-left: 8px;
						 border-bottom: 0px;
						 background-color: &&widgetBackground&& ;
						}
						QComboBox::drop-down {
						 subcontrol-position: center right;
						 border-image: url(:/gui/icons/scopy-default/icons/sba_cmb_box_arrow.svg);
						 width: 10px;
						 height: 6px;
						 font-size: 16px;
						 text-align: left;
						margin-right: 8px;
						 color: transparent;
						}
							)css");
	style.replace("&&textColor&&", RegmapStyleHelper::getColor("LabelText"));
	style.replace("&&widgetBackground&&", RegmapStyleHelper::getColor("WidgetBackground"));

	combobox->setStyleSheet(style);
}

void RegmapStyleHelper::titleSpinBoxStyle(TitleSpinBox *spinbox, QString objectName)
{
	if(!objectName.isEmpty() && spinbox)
		spinbox->setObjectName(objectName);

	QString style = QString(R"css(
						.QTitleSpinBox  {
						 font-size: 18px;
						 font-style: normal;
						 font-weight: normal;
						 text-align: left;
						 color: &&textColor&& ;
						}
						QSpinBox {
						 border-bottom: 0px;
						}
						)css");
	style.replace("&&textColor&&", RegmapStyleHelper::getColor("LabelText"));

	style += RegmapStyleHelper::grayLabelStyle();

	spinbox->setStyleSheet(style);
}

void RegmapStyleHelper::searchBarStyle(SearchBarWidget *searchBar, QString objectName)
{
	if(!objectName.isEmpty() && searchBar)
		searchBar->setObjectName(objectName);

	QString style = QString(R"css(
						QLineEdit {
						 color: &&textColor&& ;
						 font-size: 12px;
						 padding-left: 16px;
						 padding-right: 16px;
						 border-bottom: 0px;
						}
						QLineEdit:hover {
							border: 1px solid &&hoverBackground&& ;
							border-radius: 4px;
						}

						QLineEdit:disabled { color: &&disabledColor&& ; }
						QPushButton:disable {
						background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.501, y2:0, stop:0 #727273, stop:1 #141416);
						}



						)css");

	style.replace("&&textColor&&", RegmapStyleHelper::getColor("LabelText"));
	style.replace("&&hoverBackground&&", RegmapStyleHelper::getColor("ScopyBlue"));
	style.replace("&&disabledColor&&", RegmapStyleHelper::getColor("DisabledWidget"));

	style += RegmapStyleHelper::simpleWidgetStyle();

	searchBar->setStyleSheet(style);
}

void RegmapStyleHelper::smallBlueButton(QPushButton *button, QString objectName)
{
	if(!objectName.isEmpty())
		button->setObjectName(objectName);

	QString style = QString(R"css(
						QPushButton {
							 background-color: &&buttonBackground&& ;
							 border-radius: 4px;
							 font-size: 12px;
							 color: white;
						}
						QPushButton:pressed{ background-color: &&pressedColor&& ; }
						QPushButton:checked{ background-color: &&pressedColor&& ; }
						QPushButton:hover{ background-color: &&hoverColor&& ; }
						QPushButton:disabled { background-color: &&diabledColor&& ; }
						)css");

	style.replace("&&buttonBackground&&", RegmapStyleHelper::getColor("ScopyBlue"));
	style.replace("&&pressedColor&&", RegmapStyleHelper::getColor("ButtonPressed"));
	style.replace("&&hoverColor&&", RegmapStyleHelper::getColor("ButtonHover"));
	style.replace("&&diabledColor&&", RegmapStyleHelper::getColor("ButtonDisabled"));

	button->setFixedSize(16, 16);
	button->setIconSize(QSize(30, 30));
	button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	button->setStyleSheet(style);
}

QString RegmapStyleHelper::regmapControllerStyle(QWidget *widget, QString objectName)
{
	QString style = QString(R"css(
						.QWidget  {
							margin-top: 4px ;
							margin-bottom: 4px ;
							border-radius: 4px;
						}
						QLineEdit {
						 font-size: 16px;
						 font-style: normal;
						 font-weight: normal;
						 text-align: left;
						 color: &&textColor&& ;
						 border-bottom: 0px;
						}
						)css");

	style.replace("&&textColor&&", RegmapStyleHelper::getColor("LabelText"));

	style += RegmapStyleHelper::simpleWidgetStyle();

	return style;
}

QString RegmapStyleHelper::widgetidthRoundCornersStyle(QWidget *widget, QString objectName)
{
	if(!objectName.isEmpty() && widget)
		widget->setObjectName(objectName);

	QString style = QString(R"css(
						.QWidget  {
							border-radius: 4px;
						}
						)css");

	style += FrameWidgetStyle();

	if(widget) {
		widget->setStyleSheet(style);
	}

	return style;
}

QString RegmapStyleHelper::sliderStyle()
{
	QString style = QString(R"css(
						QSlider::handle {
						 background: #404040;
						 border: 0px;
						 max-width: 8px;
						 margin: -8px 0;
						 border-radius: 3px;
						}
						QSlider::handle:vertical:hover {
						 background: #4a4a4b;
						}
						QSlider::groove {
						 border: 0px;
						 background: &&grooveBackground&& ;
						 margin: 2px 0;
						 border-radius: 0px;
						 max-width: 8px;
						}
						)css");

	style.replace("&&handleBackground&&", RegmapStyleHelper::getColor("LabelText"));
	style.replace("&&grooveBackground&&", RegmapStyleHelper::getColor("Transparent"));

	return style;
}

QString RegmapStyleHelper::getColorBasedOnValue(QString value)
{
	uint32_t colorIndex = Utils::convertQStringToUint32(value) % 16;
	return Util::getColors().at(colorIndex);
}
