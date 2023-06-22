#include "regmapstylehelper.hpp"
#include <QApplication>

using namespace scopy::regmap;

RegmapStyleHelper* RegmapStyleHelper::pinstance_{nullptr};

RegmapStyleHelper::RegmapStyleHelper(QObject *parent)
{

}

RegmapStyleHelper *RegmapStyleHelper::GetInstance()
{
    if (pinstance_ == nullptr)
    {
        pinstance_ = new RegmapStyleHelper(QApplication::instance()); // singleton has the app as parent
        RegmapStyleHelper::initColorMap();
    }
    return pinstance_;
}

void RegmapStyleHelper::initColorMap() {
    auto sh = RegmapStyleHelper::GetInstance();
    sh->colorMap.insert("ScopyBlue",			"#4A64FF");
    sh->colorMap.insert("UIElementBackground",	"#272730");
    sh->colorMap.insert("LabelText",			"#FFFFFF");
    sh->colorMap.insert("ScopyBackground",		"#141416");
    sh->colorMap.insert("ButtonPressed",		"#2a44df");
    sh->colorMap.insert("ButtonHover",			"#4a34ff");
    sh->colorMap.insert("ButtonDisabled",       "grey");
}

QString RegmapStyleHelper::getColor(QString id) {
    auto sh = RegmapStyleHelper::GetInstance();
    return sh->colorMap[id];
}

void RegmapStyleHelper::BlueButton(QPushButton *btn)
{
    QString style = QString(R"css(
                        QPushButton {
                             background-color: &&buttonBackground&& ;
                             border-radius: 4px;
                             font-size: 14px;
                             color: white;
                        }
                        QPushButton:pressed{ background-color: &&pressedColor&& ; }
                        QPushButton:hover{ background-color: &&hoverColor&& ; }
                        QPushButton:disabled { background-color: &&diabledColor&& ; }

                        )css");
    style.replace("&&buttonBackground&&", RegmapStyleHelper::getColor("ScopyBlue"));
    style.replace("&&pressedColor&&", RegmapStyleHelper::getColor("ButtonPressed"));
    style.replace("&&hoverColor&&", RegmapStyleHelper::getColor("ButtonHover"));
    style.replace("&&diabledColor&&", RegmapStyleHelper::getColor("ButtonDisabled"));

    btn->setStyleSheet(style);
    btn->setFixedHeight(30);
    btn->setMinimumWidth(150);
}

void RegmapStyleHelper::BlueButton(QWidget *widget)
{
    QString style = QString(R"css(
                        QPushButton {
                             background-color: &&buttonBackground&& ;
                             border-radius: 4px;
                             font-size: 14px;
                             color: white;
                             height: 30px;
                             width: 150px;
                        }
                        QPushButton:pressed{ background-color: &&pressedColor&& ; }
                        QPushButton:hover{ background-color: &&hoverColor&& ; }
                        QPushButton:disabled { background-color: &&diabledColor&& ; }

                        )css");
    style.replace("&&buttonBackground&&", RegmapStyleHelper::getColor("ScopyBlue"));
    style.replace("&&pressedColor&&", RegmapStyleHelper::getColor("ButtonPressed"));
    style.replace("&&hoverColor&&", RegmapStyleHelper::getColor("ButtonHover"));
    style.replace("&&diabledColor&&", RegmapStyleHelper::getColor("ButtonDisabled"));

   widget->setStyleSheet(widget->styleSheet() + style);
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
                        }
                        )css");
    widget->setStyleSheet(style);
}

void RegmapStyleHelper::RegisterMapStyle(QWidget *widget)
{

  RegmapStyleHelper::BlueButton(widget);

    QString style = QString(R"css(
                        .QWidget {
                           background-color: &&widgetBackground&& ;
                        }
                        )css");
    style.replace("&&widgetBackground&&", RegmapStyleHelper::getColor("ScopyBackground"));
    widget->setStyleSheet(widget->styleSheet() + style);


}

