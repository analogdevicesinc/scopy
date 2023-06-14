#include "stylehelper.h"
#include <QApplication>

using namespace scopy;

StyleHelper* StyleHelper::pinstance_{nullptr};

StyleHelper::StyleHelper(QObject *parent)
{
}

StyleHelper *StyleHelper::GetInstance()
{
	if (pinstance_ == nullptr)
	{
		pinstance_ = new StyleHelper(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

StyleHelper::~StyleHelper()
{

}

void StyleHelper::initColorMap() {
	auto sh = StyleHelper::GetInstance();
	sh->colorMap.insert("CH0",					"#FF7200");
	sh->colorMap.insert("CH1",					"#9013FE");
	sh->colorMap.insert("CH2",					"#27B34F");
	sh->colorMap.insert("CH3",					"#F8E71C");
	sh->colorMap.insert("CH4",					"#4A64FF");
	sh->colorMap.insert("CH5",					"#02BCD4");
	sh->colorMap.insert("CH6",					"#F44336");
	sh->colorMap.insert("CH7",					"#F5A623");
	sh->colorMap.insert("ScopyBlue",			"#4A64FF");
	sh->colorMap.insert("UIElementBackground",	"#272730");
	sh->colorMap.insert("LabelText",			"#FFFFFF");
	sh->colorMap.insert("ScopyBackground",		"#141416");
}

QString StyleHelper::getColor(QString id) {
	auto sh = StyleHelper::GetInstance();
	return sh->colorMap[id];
}

void StyleHelper::SquareToggleButtonWithIcon(QPushButton *btn, QString objectName, bool checkable) {
	btn->setObjectName(objectName);
	btn->setCheckable(checkable);
	btn->setChecked(false);
	QString style = QString(R"css(
						QPushButton {
							width: 48px; /* design token - square button */
							height: 48px;
								border-radius: 0px 2px 2px 0px; /* design token - buttonBorder*/
								border-style: outset;
								background-color:#272730; /* design token - uiElement*/
						}

						QPushButton:checked {
							background-color:#4A64FF;
						}

						QPushButton:pressed {
							background-color:#4A64FF;
						}

						)css");
					btn->setStyleSheet(style);
	btn->setIconSize(QSize(48,48));
}

void StyleHelper::BlueButton(QPushButton *btn, QString objectName, bool checkable, QSize size) {
	btn->setObjectName(objectName);
	btn->setCheckable(checkable);
	btn->setChecked(false);
	btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	btn->setFixedSize(size);
	QString style = QString(R"css(
						QPushButton {
							border-radius: 0px 2px 2px 0px; /* design token - buttonBorder*/
							border-style: outset;

							background-color:#272730; /* design token - uiElement*/

						color: white; /* design token - font */
							font-weight: 700;
							font-size: 14px;
						}

						QPushButton:checked {
							background-color:#4A64FF; /* design token - scopy blue*/
						}
						QPushButton:pressed {
							background-color:#4A64FF;
						}

						)css");
					btn->setStyleSheet(style);
	btn->setIconSize(QSize(48,48));
}

void StyleHelper::BlueSquareCheckbox(QCheckBox *chk, QString objectName) {
	chk->setObjectName(objectName);
	QString style = QString(R"css(
						QCheckBox {
							spacing: 8px;
							background-color: rgba(128,128,128,0);
							font-size: 14px;
							font-weight: bold;
							color: rgba(255, 255, 255, 153);
						}
						QCheckBox::indicator {
							width: 14px;
							height: 14px;
							border: 2px solid rgb(74,100,255);
								border-radius: 4px;
						}
						QCheckBox::indicator:unchecked { background-color: black; }
						QCheckBox::indicator:checked { background-color: rgb(74,100,255); }
						)css");
					chk->setStyleSheet(style);
}

void StyleHelper::ColoredCircleCheckbox(QCheckBox *chk, QColor color, QString objectName) {
	chk->setObjectName(objectName);
	chk->setFixedSize(16,16);
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
							border-radius: 7px;
						}
						QCheckBox::indicator:unchecked { background-color: &&UIElementBackground&&; }
						QCheckBox::indicator:checked { background-color: &&colorname&&; }
						)css");
					style.replace("&&colorname&&",color.name());
					style.replace("&&UIElementBackground&&",StyleHelper::getColor("UIElementBackground"));
	chk->setStyleSheet(style);
}

void StyleHelper::CollapseCheckbox(QCheckBox *chk, QString objectName) {
	chk->setObjectName(objectName);
	chk->setFixedSize(16,16);
	QString style = QString(R"css(
						QCheckBox {
							width:16px;
							height:16px;
							background-color: rgba(128,128,128,0);
							color: rgba(255, 255, 255, 0);
						}
						QCheckBox::indicator {
							width: 12px;
							height: 12px;
							border: 2px transparent;
							background-color: transparent;
						}
						QCheckBox::indicator:unchecked {
							image: url(:/gui/icons/scopy-default/icons/sba_cmb_box_arrow_right.svg);
							background-color: transparent;
						}
						QCheckBox::indicator:checked {
							image: url(:/gui/icons/scopy-default/icons/sba_cmb_box_arrow.svg);
							background-color: transparent;
						}
						)css");
	style.replace("&&UIElementBackground&&",StyleHelper::getColor("UIElementBackground"));
	chk->setStyleSheet(style);
}

void StyleHelper::ColoredSquareCheckbox(QCheckBox *chk, QColor color, QString objectName) {
	chk->setObjectName(objectName);
	chk->setFixedSize(16,16);
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
					style.replace("&&colorname&&",color.name());
	style.replace("&&UIElementBackground&&",StyleHelper::getColor("UIElementBackground"));
	chk->setStyleSheet(style);
}

void StyleHelper::MenuControlLabel(QLabel *lbl, QString objectName) {
	lbl->setObjectName(objectName);
	lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	lbl->setMinimumWidth(80);
//	lbl->setMaximumWidth(80);
	QString style = QString(R"css(
							QLabel {
								color: white;
									background-color: rgba(255,255,255,0);
									font-weight: 700;
									font-size: 14px;
							}
							)css");
					lbl->setStyleSheet(style);
}

void StyleHelper::MenuControlButton(QPushButton *btn, QString objectName, bool checkable) {
	btn->setObjectName(objectName);
	btn->setCheckable(checkable);
	btn->setChecked(false);
	btn->setFixedSize(32,16);
	QString style = QString(R"css(
							QPushButton {
								background-color: transparent;
							}
							QPushButton#btn:pressed {
								border-image: url(:/gui/icons/setup_btn_checked.svg)
							}
							QPushButton#btn:!pressed {
								border-image: url(:/gui/icons/setup_btn_unchecked.svg)
							}
							  QPushButton#btn:hover:!pressed:!checked {
								border-image: url(:/gui/icons/setup_btn_hover.svg)
							}
							  QPushButton#btn:checked {
								border-image: url(:/gui/icons/setup_btn_checked.svg)
							}
							)css");
					btn->setStyleSheet(style);
	btn->setIconSize(QSize(48,48));
}

void StyleHelper::MenuControlWidget(QWidget *w, QColor color, QString objectName)
{
	QString style = "scopy--MenuControlButton {background-color: &&UIElementBackground&&; border-radius: 2px;}"
					"scopy--MenuControlButton[selected=true] { background-color:&&colorname&&;}";
	style.replace("&&colorname&&",color.name());
	style.replace("&&UIElementBackground&&",StyleHelper::getColor("UIElementBackground"));
	w->setStyleSheet(style);
}

