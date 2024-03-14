#include "stylehelper.h"
#include <QApplication>
#include "ui_spinbox_a.h"

#include <dynamicWidget.h>
#include <menu_anim.hpp>
#include <measurementselector.h>
#include <semiexclusivebuttongroup.h>
#include <spinbox_a.hpp>
#include <smallOnOffSwitch.h>
#include <customSwitch.h>
#include <widgets/measurementlabel.h>

using namespace scopy;

StyleHelper *StyleHelper::pinstance_{nullptr};

StyleHelper::StyleHelper(QObject *parent) {}

StyleHelper *StyleHelper::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new StyleHelper(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

StyleHelper::~StyleHelper() {}

void StyleHelper::initColorMap()
{
	auto sh = StyleHelper::GetInstance();
	sh->colorMap.insert("CH0", "#FF7200");
	sh->colorMap.insert("CH1", "#9013FE");
	sh->colorMap.insert("CH2", "#27B34F");
	sh->colorMap.insert("CH3", "#F8E71C");
	sh->colorMap.insert("CH4", "#4A64FF");
	sh->colorMap.insert("CH5", "#02BCD4");
	sh->colorMap.insert("CH6", "#F44336");
	sh->colorMap.insert("CH7", "#F5A623");
	sh->colorMap.insert("ScopyBlue", "#4A64FF");
	sh->colorMap.insert("UIElementBackground", "#272730");
	sh->colorMap.insert("UIElementHighlight", "#373740");
	sh->colorMap.insert("LabelText", "#FFFFFF");
	sh->colorMap.insert("ScopyBackground", "#141416");
	sh->colorMap.insert("ScopyBlueHover", "#4A34FF");
	sh->colorMap.insert("ScopyBlueSelected", "#3324B1");

	sh->colorMap.insert("ProgressBarIdle", "#7D7D83");
	sh->colorMap.insert("ProgressBarSuccess", "#27B34F");
	sh->colorMap.insert("ProgressBarError", "#F44336");
	sh->colorMap.insert("ProgressBarBusy", "#F8E71C");

	sh->colorMap.insert("WarningText", "#FFC904");
}

QString StyleHelper::getColor(QString id)
{
	auto sh = StyleHelper::GetInstance();
	return sh->colorMap[id];
}

void StyleHelper::SquareToggleButtonWithIcon(QPushButton *btn, QString objectName, bool checkable)
{
	if(!objectName.isEmpty())
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
	btn->setIconSize(QSize(48, 48));
}

void StyleHelper::BlueGrayButton(QPushButton *btn, QString objectName)
{
	if(!objectName.isEmpty())
		btn->setObjectName(objectName);
	btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	btn->setFixedHeight(48);
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
	btn->setIconSize(QSize(48, 48));
}

void StyleHelper::MeasurementPanelLabel(MeasurementLabel *w, QString objectName)
{
	QString style = QString(R"css(
						color: &&colorname&&;
						)css");
	style.replace(QString("&&colorname&&"), w->m_color.name());
	w->m_nameLabel->setStyleSheet(style);
	w->m_valueLabel->setStyleSheet(style);
}

void StyleHelper::StatsPanelLabel(StatsLabel *w, QString objectName)
{
	QString style = QString(R"css(
							font-weight: 500;
							font-family: Open Sans;
							font-size: 12px;
							font-style: normal;
							color: &&colorname&&;
							)css");
	style.replace(QString("&&colorname&&"), w->m_color.name());
	w->m_nameLabel->setStyleSheet(style);

	style = QString(R"css(e
							font-size: 12px;
							)css");
	style.replace(QString("&&colorname&&"), w->m_color.name());
	w->m_avgLabel->setStyleSheet(style);
	w->m_minLabel->setStyleSheet(style);
	w->m_maxLabel->setStyleSheet(style);
}

void StyleHelper::MeasurementSelectorItemWidget(QString iconPath, MeasurementSelectorItem *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QHBoxLayout *lay = dynamic_cast<QHBoxLayout *>(w->layout());
	Q_ASSERT(lay);

	lay->setMargin(0);
	lay->setSpacing(0);

	w->m_icon->setPixmap(QPixmap(iconPath));
	w->m_icon->setFixedSize(24, 24);

	w->m_name->setContentsMargins(5, 0, 0, 0);
	StyleHelper::MenuMediumLabel(w->m_name);

	//	// Add "M" letter on top of combobox
	//	QWidget *msrStack = new QWidget(w);
	//	QStackedLayout *msr_layout = new QStackedLayout(w);
	//	msrStack->setLayout(msr_layout);
	//	lay->replaceWidget(w->m_measureCheckbox, msrStack);
	//	auto measureLabel = new QLabel("M", msrStack);
	//	StyleHelper::MenuSmallLabel(measureLabel);
	//	measureLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
	//	msr_layout->setStackingMode(QStackedLayout::StackAll);
	//	msr_layout->addWidget(measureLabel);
	//	msr_layout->addWidget(w->m_measureCheckbox);

	//	auto statsLabel = new QLabel("  S", w->m_statsCheckbox);
	//	statsLabel->setFixedSize(18,18);
	//	StyleHelper::MenuSmallLabel(statsLabel);
}

void StyleHelper::TimePlotHDivInfo(QLabel *w, QString objectName)
{
	QString style = QString(R"css(
							color: &&ScopyBlue&&;
							font-weight: 500;
							font-size: 12px;
							)css");
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	w->setStyleSheet(style);
}
void StyleHelper::TimePlotSamplingInfo(QLabel *w, QString objectName)
{
	QString style = QString(R"css(
							color: rgba(192,192,192,255);
							font-weight: 400;
							font-size: 12px;
							)css");
	style.replace("&&LabelText&&", StyleHelper::getColor("LabelText"));
	w->setStyleSheet(style);
}

void StyleHelper::BlueButton(QPushButton *btn, QString objectName)
{
	if(!objectName.isEmpty())
		btn->setObjectName(objectName);
	btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	btn->setFixedHeight(36);
	QString style = QString(R"css(
						QPushButton {
							border-radius: 2px; /* design token - buttonBorder*/
							border-style: outset;

							background-color: &&ScopyBlue&&; /* design token - uiElement*/

							color: white; /* design token - font */
							font-weight: 700;
							font-size: 14px;
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

						)css");

	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	btn->setStyleSheet(style);
	btn->setIconSize(QSize(48, 48));
}

void StyleHelper::RefreshButton(QPushButton *btn, QString objectName)
{
	if(!objectName.isEmpty())
		btn->setObjectName(objectName);
	btn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	btn->setFixedSize(30, 30);
	QString style = QString(R"css(
						QPushButton {
							font-size: 12px;
							text-align: center;
							font-weight: bold;
						}
						)css");

	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	btn->setStyleSheet(style);
	btn->setProperty("blue_button", true);
	btn->setIcon(QIcon(":/gui/icons/refresh.svg"));
	btn->setIconSize(QSize(25, 25));
}

void StyleHelper::SmallBlueButton(QPushButton *btn, QString objectName)
{
	if(!objectName.isEmpty())
		btn->setObjectName(objectName);
	btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	btn->setFixedHeight(24);
	QString style = QString(R"css(
						QPushButton {
							border-radius: 2px; /* design token - buttonBorder*/
							border-style: outset;

							background-color: &&ScopyBlue&&; /* design token - uiElement*/

						color: white; /* design token - font */
							font-weight: 700;
							font-size: 14px;
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

						)css");

	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	btn->setStyleSheet(style);
	btn->setIconSize(QSize(48, 48));
}

void StyleHelper::BlueSquareCheckbox(QCheckBox *chk, QString objectName)
{
	if(!objectName.isEmpty())
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

void StyleHelper::ColoredCircleCheckbox(QCheckBox *chk, QColor color, QString objectName)
{
	if(!objectName.isEmpty())
		chk->setObjectName(objectName);
	chk->setFixedSize(16, 16);
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
	style.replace("&&colorname&&", color.name());
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	chk->setStyleSheet(style);
}

void StyleHelper::CollapseCheckbox(QCheckBox *chk, QString objectName)
{
	if(!objectName.isEmpty())
		chk->setObjectName(objectName);
	chk->setFixedSize(16, 16);
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
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	chk->setStyleSheet(style);
}

void StyleHelper::ColoredSquareCheckbox(QCheckBox *chk, QColor color, QString objectName)
{
	if(!objectName.isEmpty())
		chk->setObjectName(objectName);
	chk->setFixedSize(16, 16);
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

void StyleHelper::MenuMediumLabel(QLabel *lbl, QString objectName)
{
	if(!objectName.isEmpty())
		lbl->setObjectName(objectName);
	lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	lbl->setMinimumWidth(50);
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

void StyleHelper::MenuControlButton(QPushButton *btn, QString objectName, bool checkable)
{
	if(!objectName.isEmpty())
		btn->setObjectName(objectName);
	btn->setCheckable(checkable);
	btn->setChecked(false);
	btn->setFixedSize(32, 16);
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
	btn->setIconSize(QSize(48, 48));
}

void StyleHelper::MenuControlWidget(QWidget *w, QColor color, QString objectName)
{
	QString style = "scopy--MenuControlButton {background-color: &&UIElementBackground&&; border-radius: 2px;}"
			"scopy--MenuControlButton[selected=true] { background-color:&&colorname&&;}";
	style.replace("&&colorname&&", color.name());
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	w->setStyleSheet(style);
}

void StyleHelper::MenuComboBox(QComboBox *cb, QString objectName)
{
	if(!objectName.isEmpty())
		cb->setObjectName(objectName);
	cb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QString style = QString(R"css(
QWidget {
}
QComboBox {
 color: white;
 height: 24px;
 border: none;
 font-weight: normal;
 font-size: 16px;
 background-color: transparent;
 border-bottom: 1px solid rgba(255, 255, 255, 102);
}
QComboBox:disabled, QLineEdit:disabled { color: #555555; }
QComboBox QAbstractItemView {
 border: none;
 text-align: left;
 color: transparent;
 outline: none;
 background-color: &&ScopyBackground&&;
 selection-background-color: &&UIElementBackground&&;
 border-bottom: 1px solid #bebebe;
 border-top: 1px solid #bebebe;
}
QComboBox::item:selected {
 font-weight: bold;
 font-size: 18px;
 border-bottom: 0px solid none;
 background-color: transparent;
}
QComboBox::drop-down {
 subcontrol-position: center right;
 border-image: url(:/gui/icons/scopy-default/icons/sba_cmb_box_arrow.svg);
 width: 10px;
 height: 6px;
 font-size: 16px;
 text-align: left;
 color: transparent;
}
QComboBox::indicator {
 background-color: transparent;
 selection-background-color: transparent;
 color: transparent;
 selection-color: transparent;
}
)css");

	style.replace("&&ScopyBackground&&", StyleHelper::getColor("ScopyBackground"));
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	cb->setStyleSheet(style);
}

void StyleHelper::MenuSmallLabel(QLabel *m_lbl, QString objectName)
{
	if(!objectName.isEmpty())
		m_lbl->setObjectName(objectName);
	m_lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	QString style = QString(R"css(
				QLabel {
					color: white;
					background-color: rgba(255,255,255,0);
					font-weight: 500;
					font-family: Open Sans;
					font-size: 12px;
					font-style: normal;
					}
				QLabel:disabled {
					color: grey;
				}
				)css");
	m_lbl->setText(m_lbl->text().toUpper());
	m_lbl->setStyleSheet(style);
}

void StyleHelper::MenuComboWidget(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	w->setFixedHeight(72);
	w->layout()->setContentsMargins(10, 2, 10, 2);
	QString style = QString(R"css(
			scopy--MenuComboWidget {background-color: &&UIElementBackground&&;
			border-radius: 4px;
			}
			)css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	w->setStyleSheet(style);
}

void StyleHelper::MenuLargeLabel(QLabel *m_lbl, QString objectName)
{
	if(!objectName.isEmpty())
		m_lbl->setObjectName(objectName);
	m_lbl->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	QString style = QString(R"css(
				QLabel {
					color: white;
					background-color: rgba(255,255,255,0);
					font-weight: 700;
					font-family: Open Sans;
					font-size: 14px;
					font-style: normal;
					}
				)css");
	m_lbl->setText(m_lbl->text().toUpper());
	m_lbl->setStyleSheet(style);
}

void StyleHelper::MenuHeaderLine(QFrame *m_line, QPen pen, QString objectName)
{
	if(!objectName.isEmpty())
		m_line->setObjectName(objectName);
	m_line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_line->setFrameShape(QFrame::HLine);
	m_line->setFrameShadow(QFrame::Plain);
	m_line->setFixedHeight(2);
	//	m_line->setLineWidth(1);

	QString style = QString(R"css(
				QFrame {
					border: 2px solid &&colorname&&;
				}
				)css");
	style.replace("&&colorname&&", pen.color().name());
	m_line->setStyleSheet(style);
}

void StyleHelper::MenuHeaderWidget(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	w->setFixedHeight(48);
	w->layout()->setContentsMargins(10, 2, 10, 2);
	QString style = QString(R"css(
			scopy--MenuHeaderWidget {background-color: &&UIElementBackground&&;
			border-radius: 4px;
			}
			)css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	w->setStyleSheet(style);
}

void StyleHelper::MenuSpinComboBox(QComboBox *w, QString objectName)
{
	MenuComboBox(w, objectName);
	QString style = QString(R"css(
	QComboBox {
	font-size: 12px;
	}
	QComboBox QAbstractItemView {
	 background-color: &&ScopyBackground&&;
	 selection-background-color: &&UIElementBackground&&;
	}
	)css");

	style.replace("&&ScopyBackground&&", StyleHelper::getColor("ScopyBackground"));
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	w->setStyleSheet(style);
}

void StyleHelper::MenuOnOffSwitch(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(
	scopy--MenuOnOffSwitch {
	background-color: transparent;
	}
	)css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	w->setStyleSheet(style);
}

void StyleHelper::MenuBigSwitchButton(CustomSwitch *w, QString objectName)
{
	// This needs to be redone ...
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(
QPushButton {
min-height: 30px;
max-height: 30px;
background-color: black;
border-radius: 4px;
}

QPushButton:disabled {
background-color: qlineargradient(spread:pad, x1:0.5, y1:0, x2:0.501, y2:0, stop:0 #727273, stop:1 #141416);
}

QWidget#handle {
min-height: 30px;
max-height: 30px;
background-color: #4a64ff;
border-radius: 2px;
}

QWidget#handle:disabled {
background-color: #aaaaaa;
}

QLabel {
margin-top: 6px;
background-color: transparent;
font: 14px;
}

QLabel#on {
margin-top: 0px;
min-height:30px;
max-height:30px;
min-width:50px;
max-width:50px;
qproperty-alignment: AlignCenter AlignCenter;
margin-left: 0px;
color: white;
}

QLabel#on:disabled {
color: rgba(255,255,255,102);
}

QLabel#off {
margin-top: 0px;
min-height:30px;
max-height:30px;
min-width:50px;
max-width:50px;
margin-left: 50px;
color: white;
qproperty-alignment: AlignCenter AlignCenter;
}

QLabel#off:disabled {
color: rgba(255,255,255,51);
}
	)css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	w->setStyleSheet(style);
}

void StyleHelper::MenuLineEdit(QLineEdit *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(
QLineEdit {
 background-color: transparent;
 color: white;
 font-size: 16px;
 border: 0px solid gray;
 border-bottom: 1px solid rgba(255, 255, 255, 102);
 padding: 2px;
}
	)css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	w->setStyleSheet(style);
}

void StyleHelper::MenuLineEditWidget(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(
	scopy--MenuLineEdit {
	background-color: transparent;
	}
	)css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	w->setStyleSheet(style);
}

void StyleHelper::MenuBigSwitch(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(
	scopy--MenuBigSwitch {
	background-color: transparent;
	}
	)css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	w->setStyleSheet(style);
}

void StyleHelper::MenuCollapseSection(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);

	/// ????
	QString style = QString(R"css(
QWidget {
	background-color: transparent;
}
)css");
	/// ????
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	w->setStyleSheet(style);
}

void StyleHelper::MenuSpinBox(SpinBoxA *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);

	QString style = QString(R"css(
scopy--SpinBoxA {
	background-color: transparent;
}

scopy--SpinBoxA QPushButton#SBA_UpButton {
 width: 30px;
 height: 30px;
 border-image: url(:/gui/icons/sba_up_btn.svg);
 border: 0px;

}
scopy--SpinBoxA QPushButton#SBA_UpButton:pressed {
 border-image: url(:/gui/icons/sba_up_btn_pressed.svg);
}
scopy--SpinBoxA QPushButton#SBA_UpButton:hover:!pressed {
 border-image: url(:/gui/icons/sba_up_btn_hover.svg);
}
scopy--SpinBoxA QPushButton#SBA_DownButton {
 width: 30px;
 height: 30px;
 border-image: url(:/gui/icons/sba_dn_btn.svg);
 border: 0px;
}
scopy--SpinBoxA QPushButton#SBA_DownButton:pressed {
 border-image: url(:/gui/icons/sba_dn_btn_pressed.svg);
}
scopy--SpinBoxA QPushButton#SBA_DownButton:hover:!pressed {
 border-image: url(:/gui/icons/sba_dn_btn_hover.svg);
}
scopy--SpinBoxA QLabel#SBA_Label {
 color: rgba(255, 255, 255, 102);
 font-size: 14px;
 background-color: transparent;
}
scopy--SpinBoxA QLineEdit#SBA_LineEdit {
 height: 20px;
 width: 75px;
 font-size: 18px;
 border: 0px;
 bottom: 10px;
 background-color: transparent;
}
scopy--SpinBoxA QFrame#SBA_Line {
 height: 1px;
 background-color: transparent;
 color: &&ScopyBlue&&;
}

scopy--SpinBoxA QFrame#SBA_Line:disabled {
 color: #555555;
}

scopy--SpinBoxA QDial#SBA_CompletionCircle {
 background-color: black;
 color: &&ScopyBlue&&;
}
			)css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	w->setStyleSheet(style);
	MenuSpinComboBox(w->ui->SBA_Combobox, ""); // Should this be refactored ?
}

void StyleHelper::MenuSectionWidget(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	w->layout()->setContentsMargins(10, 10, 10, 10);
	QString style = QString(R"css(
			scopy--MenuSectionWidget {background-color: &&UIElementBackground&&;
			border-radius: 4px;
			margin-bottom: 3px;
			}
			)css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	w->setStyleSheet(style);
}

void StyleHelper::BlueIconButton(QPushButton *w, QIcon icon, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	int size = 30;
	w->setIcon(icon);
	w->setIconSize(QSize(size, size));
	w->setProperty("blue_button", true);
	w->setFixedHeight(size);
	w->setFixedWidth(size);

	QString style = QString(R"css(
			QPushButton{
			 background-color: &&ScopyBlue&&;
			 color: &&LabelText&&;
			 border-radius: 4px;
			 font-size: 14px;
			}
			QPushButton:hover:!pressed { background-color: &&ScopyBlueHover&&; }
			QPushButton:pressed { background-color: &&ScopyBlueSelected&&; }
			QPushButton:disabled { background-color: grey; }
			)css");
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	style.replace("&&LabelText&&", StyleHelper::getColor("LabelText"));
	style.replace("&&ScopyBlueHover&&", StyleHelper::getColor("ScopyBlueHover"));
	style.replace("&&ScopyBlueSelected&&", StyleHelper::getColor("ScopyBlueSelected"));
	w->setStyleSheet(style);
}

void StyleHelper::BackgroundPage(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(
		.QWidget {
			background-color: &&ScopyBackground&&;
		}
		)css");
	style.replace("&&ScopyBackground&&", StyleHelper::getColor("ScopyBackground"));
	w->setStyleSheet(style);
}

void StyleHelper::BackgroundWidget(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(
		QWidget {
			background-color: &&UIElementBackground&&;
		}
		)css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	w->setStyleSheet(style);
}

void StyleHelper::TabWidgetLabel(QLabel *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(
		QLabel {
			font-weight: bold;
			font-size: 11;
		}"
		)css");
	w->setStyleSheet(style);
}

void StyleHelper::TabWidgetEastMenu(QTabWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	w->setTabPosition(QTabWidget::TabPosition::East);
	QString style = QString(R"css(
		QTabWidget::tab-bar { left: 0; }
		QTabWidget::pane { border-top: 0px; }
		QTabBar { qproperty-drawBase: 0; }
		QTabBar::tab {
		 min-width: 100px;
		 min-height: 32px;
		 padding-bottom: 5px;
		 font: normal;
		}
		QTabBar::tab:selected {
		 color: white;
		 border-bottom: 2px solid &&ScopyBlue&&;
		 margin-top: 0px;
		}
		QTabBar::tab:!selected {
		 border-bottom: 2px solid &&UIElementHighlight&&;
		}
		QTabBar::scroller {
		 width: 25px;
		}
		)css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	style.replace("&&UIElementHighlight&&", StyleHelper::getColor("UIElementHighlight"));
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	w->tabBar()->setStyleSheet(style);
}

void StyleHelper::MenuComboLabel(QLabel *w, QString objectName) { StyleHelper::MenuSmallLabel(w, objectName); }

void StyleHelper::MenuHeaderLabel(QLabel *w, QString objectName) { StyleHelper::MenuLargeLabel(w, objectName); }

void StyleHelper::MenuControlLabel(QLabel *w, QString objectName) { StyleHelper::MenuMediumLabel(w, objectName); }

void StyleHelper::MenuOnOffSwitchLabel(QLabel *w, QString objectName) { StyleHelper::MenuSmallLabel(w, objectName); }

void StyleHelper::MenuCollapseHeaderLabel(QLabel *w, QString objectName)
{
	StyleHelper::MenuMediumLabel(w, objectName);
}

void StyleHelper::MenuOnOffSwitchButton(SmallOnOffSwitch *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
}

void StyleHelper::OverlayMenu(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(
				.QWidget {
					background-color: &&UIElementBackground&&;
					border-radius: 4px;
				})css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	w->setStyleSheet(style);
}

void StyleHelper::TutorialChapterTitleLabel(QLabel *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(
				QLabel {
					font-weight: bold;
					color: &&LabelText&&;
				}
				)css");
	style.replace("&&LabelText&&", StyleHelper::getColor("LabelText"));
	w->setStyleSheet(style);
}

void StyleHelper::DeviceIconBackgroundShadow(QAbstractButton *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);

	QString style = QString(R"css(
				QAbstractButton[selected=true] {
					background-color: #141416;
					border-radius: 4px;
				}
				)css");

	w->setStyleSheet(style);
}

void StyleHelper::FrameBackgroundShadow(QFrame *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);

	QString style = QString(R"css(
				.QFrame[selected=true] {
					background-color: #141416;
					border-radius: 4px;
				}
				)css");
	w->setStyleSheet(style);
}

void StyleHelper::HoverWidget(QWidget *w, bool draggable, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QString style;

	if(draggable) {
		style = QString(R"css(
				.QWidget {
					background-color: transparent;
					border-radius: 4px;
				}
				QWidget:hover {
					background-color: &&UIElementBackground&&;
					border-radius: 4px;
				}
				)css");
	}
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	w->setStyleSheet(style);
}

void StyleHelper::TransparentWidget(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(background-color: transparent;)css");
	w->setStyleSheet(style);
}

void StyleHelper::ScopyStatusBar(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);

	QString style = QString(R"css(
				QWidget {
					background-color: &&UIElementBackground&&;
				}
				)css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	w->setStyleSheet(style);
}

void StyleHelper::IIOWidget(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty()) {
		w->setObjectName(objectName);
	}

	w->layout()->setContentsMargins(10, 10, 10, 10);
	// background-color: &&ScopyBackground&&;

	QString style = QString(R"css(
		QFrame#&&ObjectName&& {
			border: 3px solid &&ScopyBackground&&;
			border-radius: 4px;
			margin-bottom: 3px;
		}

		QProgressBar {
			background-color: #F5A623;
		}
	)css");

	style.replace("&&ObjectName&&", objectName);
	style.replace("&&ScopyBackground&&", StyleHelper::getColor("ScopyBackground"));
	w->setStyleSheet(style);
}

void StyleHelper::IIOWidgetElement(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty()) {
		w->setObjectName(objectName);
	}

	w->layout()->setContentsMargins(10, 10, 10, 10);
	QString style = QString(R"css(
		QWidget {
			background-color: &&UIElementBackground&&;
			border-radius: 4px;
			margin-bottom: 3px;
		}

		QProgressBar {
			background-color: #F5A623;
		}
	)css");

	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	w->setStyleSheet(style);
}

void StyleHelper::IIOComboBox(QComboBox *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	QString style = QString(R"css(
QWidget {
}
QComboBox {
 color: white;
 height: 24px;
 border: none;
 font-weight: normal;
 font-size: 16px;
 background-color: transparent;
}
QComboBox:disabled, QLineEdit:disabled { color: #555555; }
QComboBox QAbstractItemView {
 border: none;
 text-align: left;
 color: transparent;
 outline: none;
 background-color: &&ScopyBackground&&;
 selection-background-color: &&UIElementBackground&&;
 border-bottom: 1px solid #bebebe;
 border-top: 1px solid #bebebe;
}
QComboBox::item:selected {
 font-weight: bold;
 font-size: 18px;
 border-bottom: 0px solid none;
 background-color: transparent;
}
QComboBox::drop-down {
 subcontrol-position: center right;
 border-image: url(:/gui/icons/scopy-default/icons/sba_cmb_box_arrow.svg);
 width: 10px;
 height: 6px;
 font-size: 16px;
 text-align: left;
 color: transparent;
}
QComboBox::indicator {
 background-color: transparent;
 selection-background-color: transparent;
 color: transparent;
 selection-color: transparent;
}
)css");

	style.replace("&&ScopyBackground&&", StyleHelper::getColor("ScopyBackground"));
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	w->setStyleSheet(style);
}

void StyleHelper::IIOLineEdit(QLineEdit *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(
QLineEdit {
 background-color: transparent;
 color: white;
 font-size: 16px;
 border: 0px solid gray;
 padding: 2px;
}
	)css");
	style.replace("&&UIElementBackground&&", StyleHelper::getColor("UIElementBackground"));
	style.replace("&&ScopyBlue&&", StyleHelper::getColor("ScopyBlue"));
	w->setStyleSheet(style);
}

void StyleHelper::TableViewWidget(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(
				QHeaderView::section {
					font: 11pt;
					border: 1px solid &&UIElementHighlight&&;
					background-color:&&ScopyBackground&&;
				}
				QTableWidget::item {
					border-right:1px dashed &&UIElementHighlight&&;
					border-bottom:1px dashed &&UIElementHighlight&&;
				}
				)css");
	style.replace("&&ScopyBackground&&", StyleHelper::getColor("ScopyBackground"));
	style.replace("&&UIElementHighlight&&", StyleHelper::getColor("UIElementHighlight"));
	w->setStyleSheet(style);
}

void StyleHelper::HoverToolTip(QWidget *w, QString info, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);

	QLabel *label = new QLabel(info);
	label->setStyleSheet("QLabel {"
			     "	font-weight: bold;"
			     "	color: #FFFFFF;"
			     "}");

	w->setStyleSheet("QWidget {"
			 "	background-color: #272730;"
			 "}");

	QHBoxLayout *layout = new QHBoxLayout(w);
	layout->addWidget(label);
}
void StyleHelper::WarningLabel(QLabel *w, QString objectName)
{
	QString style = QString(R"css(
					color: &&WarningText&&;
					font-size: 11pt;
							)css");
	style.replace("&&WarningText&&", StyleHelper::getColor("WarningText"));
	w->setStyleSheet(style);
}

void StyleHelper::NoBackgroundIconButton(QPushButton *w, QIcon icon, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	int size = 30;
	w->setIcon(icon);
	w->setIconSize(QSize(size, size));
	w->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	QString style = QString(R"css(
			QPushButton{
				background-color: none;
				border: 1px;
			}
			)css");
	w->setStyleSheet(style);
}

void StyleHelper::OrangeWidget(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	QString style = QString(R"css(
			QWidget{
				color:&&orange&&
			}
			)css");
	style.replace("&&orange&&", StyleHelper::getColor("CH0"));
	w->setStyleSheet(style);
}

#include "moc_stylehelper.cpp"
