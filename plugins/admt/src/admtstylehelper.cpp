/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "admtstylehelper.h"
#include "stylehelper.h"
#include <style.h>

using namespace scopy::admt;

ADMTStyleHelper *ADMTStyleHelper::pinstance_{nullptr};

ADMTStyleHelper::ADMTStyleHelper(QObject *parent) {}

ADMTStyleHelper *ADMTStyleHelper::GetInstance() {
  if (pinstance_ == nullptr) {
    pinstance_ = new ADMTStyleHelper(
        QApplication::instance()); // singleton has the app as parent
  }
  return pinstance_;
}

ADMTStyleHelper::~ADMTStyleHelper() {}

void ADMTStyleHelper::initColorMap() {
  auto sh = ADMTStyleHelper::GetInstance();
  sh->colorMap.insert("CH0", "#FF7200");
  sh->colorMap.insert("CH1", "#9013FE");
  sh->colorMap.insert("CH2", "#27B34F");
  sh->colorMap.insert("CH3", "#F8E71C");
  sh->colorMap.insert("CH4", "#4A64FF");
  sh->colorMap.insert("CH5", "#02BCD4");
  sh->colorMap.insert("CH6", "#F44336");
  sh->colorMap.insert("CH7", "#F5A623");
  sh->colorMap.insert("CH8", "#1981AE");
  sh->colorMap.insert("CH9", "#6FCEA6");
  sh->colorMap.insert("CH10", "#F7A1DA");
  sh->colorMap.insert("CH11", "#E3F5FC");
}

QString ADMTStyleHelper::getColor(QString id) {
  auto sh = ADMTStyleHelper::GetInstance();
  return sh->colorMap[id];
}

void ADMTStyleHelper::TopContainerButtonStyle(QPushButton *btn,
                                              QString objectName) {
  if (!objectName.isEmpty())
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
  style.replace("&&ScopyBlue&&",
                Style::getAttribute(json::theme::interactive_primary_idle));
  btn->setStyleSheet(style);
}

void ADMTStyleHelper::PlotWidgetStyle(PlotWidget *widget, QString objectName) {
  if (!objectName.isEmpty())
    widget->setObjectName(objectName);
  widget->setContentsMargins(10, 10, 10, 6);
  widget->plot()->canvas()->setStyleSheet("background-color: black;");
}

void ADMTStyleHelper::ComboBoxStyle(QComboBox *widget, QString objectName) {
  if (!objectName.isEmpty())
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
  style = style.replace(QString("&&colorname&&"),
                        Style::getAttribute(json::global::ch0));
  widget->setStyleSheet(style);
  widget->setFixedHeight(30);
}

void ADMTStyleHelper::LineEditStyle(QLineEdit *widget, QString objectName) {
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
  style = style.replace(QString("&&colorname&&"),
                        Style::getAttribute(json::global::ch0));
  widget->setStyleSheet(style);
  widget->setFixedHeight(30);
  widget->setContentsMargins(0, 0, 0, 0);
  widget->setTextMargins(12, 4, 12, 4);
  widget->setAlignment(Qt::AlignRight);
}

void ADMTStyleHelper::ColoredSquareCheckbox(QCheckBox *chk, QColor color,
                                            QString objectName) {
  if (!objectName.isEmpty())
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
  style.replace("&&UIElementBackground&&",
                Style::getAttribute(json::theme::background_primary));
  chk->setStyleSheet(style);
}

void ADMTStyleHelper::StartButtonStyle(QPushButton *btn, QString objectName) {
  if (!objectName.isEmpty())
    btn->setObjectName(objectName);
  QString style = QString(R"css(
			QPushButton {
				border-radius: 2px;
				padding-left: 20px;
				padding-right: 20px;
				color: white;
				font-weight: 700;
				font-size: 14px;
			}

			QPushButton:!checked {
				background-color: #27b34f;
			}

			  QPushButton:checked {
				background-color: #F45000;
			}

			QPushButton:disabled {
				background-color: grey;
			})css");
  btn->setCheckable(true);
  btn->setChecked(false);
  btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  btn->setFixedHeight(36);
  btn->setStyleSheet(style);
  QIcon playIcon;
  playIcon.addPixmap(Util::ChangeSVGColor(":/gui/icons/play.svg", "white", 1),
                     QIcon::Normal, QIcon::Off);
  playIcon.addPixmap(
      Util::ChangeSVGColor(":/gui/icons/scopy-default/icons/play_stop.svg",
                           "white", 1),
      QIcon::Normal, QIcon::On);
  btn->setIcon(playIcon);
  btn->setIconSize(QSize(64, 64));
}

void ADMTStyleHelper::TextStyle(QWidget *widget, const char *styleHelperColor,
                                bool isBold, QString objectName) {
  if (!objectName.isEmpty())
    widget->setObjectName(objectName);
  QString existingStyle = widget->styleSheet();
  QString style = QString(R"css(
								font-size: 16px;
								font-weight: &&fontweight&&;
								text-align: right;
								color: &&colorname&&;
							)css");
  style = style.replace(QString("&&colorname&&"),
                        Style::getAttribute(styleHelperColor));
  QString fontWeight = QString("normal");
  if (isBold) {
    fontWeight = QString("bold");
  }
  style = style.replace(QString("&&fontweight&&"), fontWeight);
  widget->setStyleSheet(existingStyle + style);
}

void ADMTStyleHelper::MenuSmallLabel(QLabel *label, QString objectName) {
  if (!objectName.isEmpty())
    label->setObjectName(objectName);
  label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

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
  label->setStyleSheet(style);
}

void ADMTStyleHelper::LineStyle(QFrame *line, QString objectName) {
  if (!objectName.isEmpty())
    line->setObjectName(objectName);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Plain);
  line->setFixedHeight(1);
  QString lineStyle = QString(R"css(
				QFrame {
					border: 1px solid #808085;
				}
				)css");
  line->setStyleSheet(lineStyle);
}

void ADMTStyleHelper::UIBackgroundStyle(QWidget *widget, QString objectName) {
  if (!objectName.isEmpty())
    widget->setObjectName(objectName);
  QString style = QString(R"css(
								background-color: &&colorname&&;
							)css");
  style.replace(QString("&&colorname&&"),
                Style::getAttribute(json::theme::background_primary));
  widget->setStyleSheet(style);
}

void ADMTStyleHelper::GraphChannelStyle(QWidget *widget, QLayout *layout,
                                        QString objectName) {
  if (!objectName.isEmpty())
    widget->setObjectName(objectName);
  widget->setLayout(layout);
  ADMTStyleHelper::UIBackgroundStyle(widget);
  layout->setContentsMargins(20, 13, 20, 5);
  layout->setSpacing(20);
}

void ADMTStyleHelper::CalculatedCoeffWidgetRowStyle(
    QWidget *widget, QHBoxLayout *layout, QLabel *hLabel, QLabel *hMagLabel,
    QLabel *hPhaseLabel, QString objectName) {
  if (!objectName.isEmpty())
    widget->setObjectName(objectName);

  widget->setLayout(layout);
  QString style = QString(R"css(
								background-color: &&colorname&&;
								border-radius: 4px;
							)css");
  style.replace(QString("&&colorname&&"),
                Style::getAttribute(json::theme::background_subtle));
  widget->setStyleSheet(style);
  widget->setFixedHeight(30);
  widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  layout->setContentsMargins(12, 4, 12, 4);

  ADMTStyleHelper::TextStyle(hLabel, json::global::white, true);
  ADMTStyleHelper::TextStyle(hMagLabel, json::global::ch0);
  ADMTStyleHelper::TextStyle(hPhaseLabel, json::global::ch1);

  hLabel->setFixedWidth(24);
  hMagLabel->setContentsMargins(0, 0, 32, 0);
  hPhaseLabel->setFixedWidth(72);

  layout->addWidget(hLabel);
  layout->addWidget(hMagLabel, 0, Qt::AlignRight);
  layout->addWidget(hPhaseLabel);
}

#include "moc_admtstylehelper.cpp"