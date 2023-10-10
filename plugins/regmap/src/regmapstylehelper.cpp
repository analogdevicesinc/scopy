#include "regmapstylehelper.hpp"

#include "dynamicWidget.h"

#include <QApplication>
#include <qboxlayout.h>

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

void RegmapStyleHelper::FrameWidget(QWidget *widget)
{
	QString style = QString(R"css(
                        .QWidget {
                                 border-top: 2px solid black;
                                 border-right: 2px solid black;
                                 border-left: 2px solid black;
                                 border-bottom: 2px solid black;
                                 background-color: &&background&& ;
                        }
                        )css");
	style.replace("&&background&&", RegmapStyleHelper::getColor("WidgetBackground"));
	widget->setStyleSheet(style);
}

void RegmapStyleHelper::SettingsMenu(QWidget *widget)
{
	QString style = QString(R"css(
                        .QWidget {
                                 background-color: &&background&& ;
                        }
                        QWidget {
                                 font: normal;
                                 color: &&textColor&& ;
                                 font-size: 16px;
                        }
                        )css");

	style.replace("&&background&&", RegmapStyleHelper::getColor("ScopyBackground"));
	style.replace("&&textColor&&", RegmapStyleHelper::getColor("LabelText"));

	widget->setStyleSheet(style);
}

void RegmapStyleHelper::RegisterMapStyle(QWidget *widget)
{
	QString style = QString(R"css(
                        .QWidget {
                           background-color: &&widgetBackground&& ;
                        }
                        )css");
	style.replace("&&widgetBackground&&", RegmapStyleHelper::getColor("ScopyBackground"));
	style += RegmapStyleHelper::BlueButton(nullptr);
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
                         font-size: 13px;
                         font-style: normal;
                         font-weight: normal;
                         text-align: left;
                         color: &&labelColor&& ;
                        }
                        )css");
	style.replace("&&labelColor&&", RegmapStyleHelper::getColor("LabelText2"));
	label->setStyleSheet(style);
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

                        .QFrame {
                                background-color: &&frameBackground&& ;
                                border-radius: 4px;
                        }
                        QWidget {
                                background-color: &&childWidgetBackground&& ;
                        }

                        )css");

	style.replace("&&frameBackground&&", RegmapStyleHelper::getColor("WidgetBackground"));
	style.replace("&&childWidgetBackground&&", RegmapStyleHelper::getColor("Transparent"));

	widget->setMinimumWidth(10);
	widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

	widget->nameLabel->setStyleSheet(whiteSmallTextLable(nullptr));
	if(widget->valueCheckBox) {
		widget->valueCheckBox->setStyleSheet(checkboxStyle(nullptr));
	}
	widget->setFixedHeight(120);

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
