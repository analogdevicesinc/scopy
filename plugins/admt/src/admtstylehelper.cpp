#include "admtstylehelper.h"
#include "stylehelper.h"

using namespace scopy::admt;

ADMTStyleHelper *ADMTStyleHelper::pinstance_{nullptr};

ADMTStyleHelper:: ADMTStyleHelper(QObject *parent) {}

ADMTStyleHelper *ADMTStyleHelper::GetInstance()
{
    if(pinstance_ == nullptr) {
        pinstance_  = new ADMTStyleHelper(QApplication::instance()); // singleton has the app as parent
    }
    return pinstance_;
}

ADMTStyleHelper::~ADMTStyleHelper() {}

void ADMTStyleHelper::TopContainerButtonStyle(QPushButton *btn, QString objectName)
{
    if(!objectName.isEmpty())
		btn->setObjectName(objectName);
    QString style = QString(R"css(
			QPushButton {
				width: 88px;
				height: 48px;
				border-radius: 2px;
				padding-left: 20px;
				padding-right: 20px;
				color: white;
				font-weight: 700;
				font-size: 14px;
				background-color: &&ScopyBlue&&;
			}

			QPushButton:disabled {
				background-color:#727273; /* design token - uiElement*/
			}

			QPushButton:checked {
				background-color:#272730; /* design token - scopy blue*/
			}
			QPushButton:pressed {
				background-color:#272730;
			}
			})css");
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
    btn->setStyleSheet(style);
}

void ADMTStyleHelper::PlotWidgetStyle(PlotWidget *widget, QString objectName)
{
    if(!objectName.isEmpty())
		widget->setObjectName(objectName);
    widget->setContentsMargins(10, 10, 10, 6);
	widget->plot()->canvas()->setStyleSheet("background-color: black;");
}

void ADMTStyleHelper::ComboBoxStyle(QComboBox *widget, QString objectName)
{
    if(!objectName.isEmpty())
		widget->setObjectName(objectName);
	QString style = QString(R"css(
        QWidget {
        }
        QComboBox {
            text-align: right;
            color: &&colorname&&;
            border-radius: 4px;
            height: 30px;
            border-bottom: 0px solid none;
            padding-left: 12px;
            padding-right: 12px;
            font-weight: normal;
            font-size: 16px;
            background-color: black;
        }
        QComboBox:disabled, QLineEdit:disabled {
            background-color: #18181d;
            color: #9c4600;
        }
        QComboBox QAbstractItemView {
            border: none;
            color: transparent;
            outline: none;
            background-color: black;
            border-bottom: 0px solid transparent;
            border-top: 0px solid transparent;
        }
        QComboBox QAbstractItemView::item {
            text-align: right;
        }
        QComboBox::item:selected {
            font-weight: bold;
            font-size: 18px;
            background-color: transparent;
        }
        QComboBox::drop-down  {
            border-image: none;
            border: 0px;
            width: 16px;
            height: 16px;
            margin-right: 12px;
        }
        QComboBox::down-arrow {
            image: url(:/admt/chevron-down-s.svg);
        }
        QComboBox::indicator {
            background-color: transparent;
            selection-background-color: transparent;
            color: transparent;
            selection-color: transparent;
        }
    )css");
	style = style.replace(QString("&&colorname&&"), StyleHelper::getColor("CH0"));
	widget->setStyleSheet(style);
	widget->setFixedHeight(30);
}

void ADMTStyleHelper::LineEditStyle(QLineEdit *widget, QString objectName)
{
	QString style = QString(R"css(
								QLineEdit {
									font-family: Open Sans;
									font-size: 16px;
									font-weight: normal;
									text-align: right;
									color: &&colorname&&;

									background-color: black;
									border-radius: 4px;
									border: none;
								}

								QLineEdit:disabled {
                                    background-color: #18181d;
									color: #9c4600;
                                }
							)css");
	style = style.replace(QString("&&colorname&&"), StyleHelper::getColor("CH0"));
	widget->setStyleSheet(style);
	widget->setFixedHeight(30);
	widget->setContentsMargins(0, 0, 0, 0);
	widget->setTextMargins(12, 4, 12, 4);
	widget->setAlignment(Qt::AlignRight);
}

void ADMTStyleHelper::ColoredSquareCheckbox(QCheckBox *chk, QColor color, QString objectName)
{
	if(!objectName.isEmpty())
		chk->setObjectName(objectName);
	QString style = QString(R"css(
						QCheckBox {
							width:16px;
							height:16px;
							background-color: rgba(128,128,128,0);
							color: rgba(255, 255, 255, 153);
						}
						QCheckBox::indicator {
							width: 12px;
							height: 12px;
							border: 2px solid #FFFFFF;
							border-radius: 4px;
						}
						QCheckBox::indicator:unchecked { background-color: &&UIElementBackground&&; }
						QCheckBox::indicator:checked { background-color: &&colorname&&; }
						)css");
	style.replace("&&colorname&&", color.name());
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	chk->setStyleSheet(style);
}

#include "moc_admtstylehelper.cpp"