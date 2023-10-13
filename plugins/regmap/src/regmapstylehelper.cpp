#include "regmapstylehelper.hpp"

#include "dynamicWidget.h"
#include <QComboBox>
#include <QLineEdit>
#include <qboxlayout.h>
#include <registermapsettingsmenu.hpp>
#include <stylehelper.h>
#include <QApplication>

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
	sh->colorMap.insert("ScopyBlue", "#4A64FF");
	sh->colorMap.insert("UIElementBackground", "#272730");
	sh->colorMap.insert("LabelText", "#FFFFFF");
	sh->colorMap.insert("LabelText2", "rgba(255, 255, 255, 150)");
	sh->colorMap.insert("ScopyBackground", "#141416");
	sh->colorMap.insert("WidgetBackground", "#272730");
	sh->colorMap.insert("ButtonPressed", "#2a44df");
	sh->colorMap.insert("ButtonHover", "#4a34ff");
	sh->colorMap.insert("ButtonDisabled", "grey");
	sh->colorMap.insert("Red", "red");
	sh->colorMap.insert("Transparent", "transparent");
	sh->colorMap.insert("CheckBoxTextColor", "rgba(255, 255, 255, 153)");
	sh->colorMap.insert("CheckBoxIndicatorBorderColor", "rgb(74,100,255)");
	sh->colorMap.insert("ScopyOrange", "#FF7200");
}

QString RegmapStyleHelper::getColor(QString id)
{
	auto sh = RegmapStyleHelper::GetInstance();
	return sh->colorMap[id];
}

void RegmapStyleHelper::PartialFrameWidget(QWidget *widget)
{
	QString style = QString(R"css(
                        .QWidget {
                                 border-top: 2px solid black;
                                 border-right: 2px solid black;
                                 border-left: 2px solid black;
                        }
                        )css");
	widget->setStyleSheet(style);
}

QString RegmapStyleHelper::FrameWidget(QWidget *widget)
{
	QString style = QString(R"css(
                        .QWidget {
                                 border-top: 2px solid black;
                                 border-right: 2px solid black;
                                 border-left: 2px solid black;
                                 border-bottom: 2px solid black;
                                 background-color: &&background&& ;
                        }

                        QWidget  {
                            background-color: &&background&& ;
                        }

                        )css");
	style.replace("&&background&&", RegmapStyleHelper::getColor("WidgetBackground"));

	if(widget) {
		widget->setStyleSheet(style);
	}
	return style;
}

void RegmapStyleHelper::RegisterMapStyle(QWidget *widget)
{
	QString style = QString(R"css(
                        .QWidget {
                           background-color: &&widgetBackground&& ;
                        }
                        )css");
	style.replace("&&widgetBackground&&", RegmapStyleHelper::getColor("ScopyBackground"));
	widget->setStyleSheet(style);
}

void RegmapStyleHelper::bigTextLabelStyle(QLabel *label, QString objectName)
{
	if(!objectName.isEmpty())
		label->setObjectName(objectName);

	QString style = QString(R"css(
                        QLabel {
                        font: normal;
                        font-size: 16px;
                        color: &&labelColor&& ;
                        }
                        )css");
	style.replace("&&labelColor&&", RegmapStyleHelper::getColor("LabelText"));
	label->setStyleSheet(style);
}

QString RegmapStyleHelper::BlueButton(QPushButton *btn, QString objectName)
{
	if(!objectName.isEmpty())
		btn->setObjectName(objectName);

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

	if(btn) {
		btn->setStyleSheet(style);
	}

	return style;
}

QString RegmapStyleHelper::checkboxStyle(QCheckBox *checkbox, QString objectName)
{
	if(!objectName.isEmpty())
		checkbox->setObjectName(objectName);

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

	if(checkbox) {
		checkbox->setStyleSheet(style);
	}
	return style;
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

QString RegmapStyleHelper::regmapSettingsMenu(RegisterMapSettingsMenu *settings, QString objectName)
{
	if(!objectName.isEmpty() && settings)
		settings->setObjectName(objectName);

	QString style = QString(R"css(
                        QWidget {
                                 background-color: &&background&& ;
                        }
                        QWidget {
                                 font: normal;
                                 color: &&textColor&& ;
                                 font-size: 16px;
                        }
                        )css");

	style.replace("&&background&&", RegmapStyleHelper::getColor("Transparent"));
	style.replace("&&textColor&&", RegmapStyleHelper::getColor("LabelText"));

	RegmapStyleHelper::checkboxStyle(settings->autoread, "");
	RegmapStyleHelper::bigTextLabelStyle(settings->hexaPrefix1, "");
	RegmapStyleHelper::bigTextLabelStyle(settings->hexaPrefix2, "");
	RegmapStyleHelper::BlueButton(settings->readInterval, "");
	RegmapStyleHelper::BlueButton(settings->writeListOfValuesButton, "");
	RegmapStyleHelper::BlueButton(settings->registerDump, "");
	RegmapStyleHelper::BlueButton(settings->pathButton, "");

	settings->setStyleSheet(style);

	return style;
}

QString RegmapStyleHelper::grayBackgroundHoverWidget(QWidget *widget, QString objectName)
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

	if(widget) {
		widget->setStyleSheet(style);
	}
	return style;
}

QString RegmapStyleHelper::detailedBitFieldStyle(BitFieldDetailedWidget *widget, QString objectName)
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

	widget->nameLabel->setStyleSheet(whiteSmallTextLable(nullptr));
	if(widget->valueSwitch) {
		StyleHelper::MenuOnOffSwitchButton(dynamic_cast<SmallOnOffSwitch *>(widget->valueSwitch),
						   "onOffSwitch");
	}
	widget->setFixedHeight(96);

	return style;
}

QString RegmapStyleHelper::simpleRegisterStyle(RegisterSimpleWidget *widget, QString objectName)
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
                        )css");

	style.replace("&&frameBackground&&", RegmapStyleHelper::getColor("WidgetBackground"));
	style.replace("&&childWidgetBackground&&", RegmapStyleHelper::getColor("Transparent"));
	style.replace("&&hoverBackground&&", RegmapStyleHelper::getColor("ButtonHover"));
	style.replace("&&selectedBackground&&", RegmapStyleHelper::getColor("ScopyBlue"));
	widget->setStyleSheet(style);
	widget->regBaseInfoWidget->setStyleSheet(grayBackgroundHoverWidget(nullptr));
	widget->registerNameLabel->setStyleSheet(whiteSmallTextLable(nullptr));

	return style;
}

QString RegmapStyleHelper::valueLabel(QLabel *label, QString objectName)
{
	if(!objectName.isEmpty())
		label->setObjectName(objectName);

	QString style = QString(R"css(
                        *[value_label=true] {

                             font-size: 12px;
                             font-style: normal;
                             font-weight: normal;
                             text-align: left;
                             color: &&labelColor&& ;

                        }
                        )css");
	style.replace("&&labelColor&&", RegmapStyleHelper::getColor("ScopyOrange"));

	return style;
}

QString RegmapStyleHelper::grayLabel(QLabel *label, QString objectName)
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

	return style;
}

QString RegmapStyleHelper::whiteSmallTextLable(QLabel *label, QString objectName)
{
	if(!objectName.isEmpty())
		label->setObjectName(objectName);

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

QString RegmapStyleHelper::frameBorderHover(QFrame *frame, QString objectName)
{
	if(!objectName.isEmpty() && frame)
		frame->setObjectName(objectName);

	QString style = QString(R"css(
                        .scopy--regmap--RegisterSimpleWidget:hover  {
                            border: 1px solid &&hoverBackground&& ;
                            border-radius: 4px;
                        }
                        )css");
	style.replace("&&hoverBackground&&", RegmapStyleHelper::getColor("LabelText"));

	return style;
}

QString RegmapStyleHelper::simpleWidgetWithButtonStyle(QWidget *widget, QString objectName)
{
	if(!objectName.isEmpty() && widget)
		widget->setObjectName(objectName);

	QString style = "";
	style += RegmapStyleHelper::simpleWidgetStyle(nullptr);
	style += RegmapStyleHelper::BlueButton(nullptr);

	return style;
}

QString RegmapStyleHelper::simpleWidgetStyle(QWidget *widget, QString objectName)
{
	if(!objectName.isEmpty() && widget)
		widget->setObjectName(objectName);

	QString style = QString(R"css(
                        QWidget  {
                            background-color: &&widgetBackground&& ;
							border-radius: 4px;
                        }
                        )css");
	style.replace("&&widgetBackground&&", RegmapStyleHelper::getColor("WidgetBackground"));

	return style;
}

QString RegmapStyleHelper::comboboxStyle(QComboBox *combobox, QString objectName)
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

	return style;
}

QString RegmapStyleHelper::lineEditStyle(QLineEdit *lineEdit, QString objectName)
{
	if(!objectName.isEmpty() && lineEdit)
		lineEdit->setObjectName(objectName);

	QString style = QString(R"css(
						QLineEdit  {
						 font-size: 18px;
						 font-style: normal;
						 font-weight: normal;
						 text-align: left;
						 color: &&textColor&& ;
						}
						)css");

	style.replace("&&textColor&&", RegmapStyleHelper::getColor("LabelText"));

	return style;
}

QString RegmapStyleHelper::spinboxStyle(QSpinBox *spinbox, QString objectName)
{
	if(!objectName.isEmpty() && spinbox)
		spinbox->setObjectName(objectName);

	QString style = QString(R"css(
						QSpinBox  {
						 font-size: 18px;
						 font-style: normal;
						 font-weight: normal;
						 text-align: left;
						 color: &&textColor&& ;
						}
						)css");

	style.replace("&&textColor&&", RegmapStyleHelper::getColor("LabelText"));

	return style;
}

QString RegmapStyleHelper::titleSpinBoxStyle(TitleSpinBox *spinbox, QString objectName)
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

	style += grayLabel(nullptr);

	return style;
}

QString RegmapStyleHelper::searchBarStyle(SearchBarWidget *searchBar, QString objectName)
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
						)css");
	style.replace("&&textColor&&", RegmapStyleHelper::getColor("LabelText"));
	style.replace("&&hoverBackground&&", RegmapStyleHelper::getColor("ScopyBlue"));

	style += simpleWidgetStyle(nullptr);
	return style;
}

QString RegmapStyleHelper::smallBlueButton(QPushButton *button, QString objectName)
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

	if(button) {
		button->setStyleSheet(style);
		button->setFixedSize(16, 16);
		button->setIconSize(QSize(30, 30));
		button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	}
	return style;
}

QString RegmapStyleHelper::regmapControllerStyle(QWidget *widget, QString objectName)
{
	//	if (!objectName.isEmpty() && widget) widget->setObjectName(objectName);

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

	style += simpleWidgetStyle(widget);

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

	style += FrameWidget(widget);

	if(widget) {
		widget->setStyleSheet(style);
	}

	return style;
}
