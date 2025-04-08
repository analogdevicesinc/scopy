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

#include "stylehelper.h"
#include <QApplication>

#include <dynamicWidget.h>
#include <menu_anim.hpp>
#include <measurementselector.h>
#include <semiexclusivebuttongroup.h>
#include <spinbox_a.hpp>
#include <smallOnOffSwitch.h>
#include <customSwitch.h>
#include <style.h>
#include <stylehelper.h>
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

QString StyleHelper::getChannelColor(int index)
{
	QColor color;
	int colorCount = Style::getChannelColorList().length();

	if(colorCount != 0 && colorCount <= index) {
		color = QColor(getChannelColor(index - colorCount));
		color.setHsv(color.hue() + 30, color.saturation(), color.value());
	} else {
		color = QColor(Style::getChannelColor(index));
	}

	return color.name();
}

void StyleHelper::MeasurementPanelLabel(MeasurementLabel *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);

	QString style = QString(R"css(
						color: &&colorname&&;
						)css");
	style.replace(QString("&&colorname&&"), w->m_color.name());
	w->m_nameLabel->setStyleSheet(style);
	w->m_valueLabel->setStyleSheet(style);
}

void StyleHelper::StatsPanelLabel(StatsLabel *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);

	QString style = QString(R"css(
				font-size: &&font_size_0_5&&;
				color: &&colorname&&;
				)css");
	style.replace(QString("&&colorname&&"), w->m_color.name());
	style.replace(QString("&&font_size_0_5&&"), Style::getAttribute(json::global::font_size_0_5));
	w->m_nameLabel->setStyleSheet(style);

	style = QString(R"css(
				font-size: &&font_size_0_5&&;
				)css");
	style.replace(QString("&&font_size_0_5&&"), Style::getAttribute(json::global::font_size_0_5));
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

	w->m_icon->setPixmap(iconPath);
	w->m_icon->setFixedSize(24, 24);

	w->m_name->setContentsMargins(5, 0, 0, 0);
	Style::setStyle(w->m_name, style::properties::label::menuMedium);
}

void StyleHelper::BasicButton(QPushButton *btn, QString objectName)
{
	if(!objectName.isEmpty())
		btn->setObjectName(objectName);

	Style::setStyle(btn, style::properties::button::basicButton);
	btn->setFixedHeight(Style::getDimension(json::global::unit_4));
}

void StyleHelper::RefreshButton(QPushButton *btn, QString objectName)
{
	if(!objectName.isEmpty())
		btn->setObjectName(objectName);

	Style::setStyle(btn, style::properties::button::basicButton);
	btn->setIcon(Style::getPixmap(":/gui/icons/refresh.svg", Style::getColor(json::theme::content_inverse)));

	int icon_size = Style::getAttribute(json::global::unit_2).toInt();
	int size = Style::getAttribute(json::global::unit_2_5).toInt();
	btn->setIconSize(QSize(icon_size, icon_size));
	btn->setFixedSize(QSize(size, size));
}

void StyleHelper::BasicSmallButton(QPushButton *btn, QString objectName)
{
	if(!objectName.isEmpty())
		btn->setObjectName(objectName);

	Style::setStyle(btn, style::properties::button::basicButton, true, true);
	btn->setFixedHeight(Style::getDimension(json::global::unit_3));
}

void StyleHelper::ColoredCircleCheckbox(QCheckBox *chk, QColor color, QString objectName)
{
	if(!objectName.isEmpty())
		chk->setObjectName(objectName);

	chk->setStyleSheet("QCheckBox::indicator:checked { background-color: " + color.name() + "; }");
	Style::setStyle(chk, style::properties::checkbox::circleCB, "idle", true);
}

void StyleHelper::CollapseCheckbox(QCheckBox *chk, QString objectName)
{
	if(!objectName.isEmpty())
		chk->setObjectName(objectName);

	int size = Style::getDimension(json::global::unit_1);
	chk->setFixedSize(size, size);
	Style::setStyle(chk, style::properties::checkbox::collapseCB, true, true);
}

void StyleHelper::ColoredSquareCheckbox(QCheckBox *chk, QColor color, QString objectName)
{
	if(!objectName.isEmpty())
		chk->setObjectName(objectName);

	chk->setStyleSheet("QCheckBox::indicator:checked { background-color: " + color.name() + "; }");
	Style::setStyle(chk, style::properties::checkbox::squareCB, true, true);
}

void StyleHelper::MenuControlButton(QPushButton *btn, QString objectName, bool checkable)
{
	if(!objectName.isEmpty())
		btn->setObjectName(objectName);
	btn->setCheckable(checkable);
	btn->setChecked(false);
	btn->setFixedSize(Style::getDimension(json::global::unit_2_5), Style::getDimension(json::global::unit_1));
	Style::setStyle(btn, style::properties::button::menuControlButton, true, true);

	int size = Style::getDimension(json::global::unit_4);
	btn->setIconSize(QSize(size, size));
}

void StyleHelper::MenuControlWidget(QWidget *w, QColor color, QString objectName)
{
	QString style = "scopy--MenuControlButton {background-color: &&background_primary&&;}"
			"scopy--MenuControlButton[selected=true] { background-color:&&colorname&&;}";
	style.replace("&&colorname&&", color.name());
	style.replace("&&background_primary&&", Style::getAttribute(json::theme::background_primary));
	w->setStyleSheet(style);
}

void StyleHelper::MenuHeaderLine(QFrame *m_line, QPen pen, QString objectName)
{
	if(!objectName.isEmpty())
		m_line->setObjectName(objectName);
	m_line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	m_line->setFrameShape(QFrame::HLine);
	m_line->setFrameShadow(QFrame::Plain);
	m_line->setFixedHeight(1);

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

	MenuSectionWidget(w);
	w->setFixedHeight(Style::getDimension(json::global::unit_4));
}

void StyleHelper::MenuSectionWidget(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);

	w->layout()->setContentsMargins(10, 10, 10, 10);
	Style::setStyle(w, style::properties::widget::solidBorder, true, true);
	Style::setStyle(w, style::properties::widget::basicBackground, true, true);
}

void StyleHelper::BlueIconButton(QPushButton *w, QIcon icon, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	int size = Style::getDimension(json::global::unit_2_5);
	w->setIcon(icon);
	w->setIconSize(QSize(size, size));
	w->setFixedHeight(size);
	w->setFixedWidth(size);

	Style::setStyle(w, style::properties::button::basicButton, true, true);
}

void StyleHelper::BackgroundWidget(QWidget *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	Style::setBackgroundColor(w, json::theme::background_primary);
}

void StyleHelper::MenuCollapseHeaderLineEdit(QLineEdit *w, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	w->setMinimumWidth(50);

	Style::setStyle(w, style::properties::lineedit::headerLineEdit);
}

void StyleHelper::HoverWidget(QWidget *w, bool draggable, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);
	w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QString style;

	style = QString(R"css(
				QWidget {
					background-color: &&Background&&;
					border-radius: 4px;
				}
				QWidget:hover {
					background-color: &&HoverBackground&&;
					border-radius: 4px;
				}
				)css");

	if(draggable) {
		style.replace("&&Background&&", Style::getAttribute(json::theme::background_primary));
		style.replace("&&HoverBackground&&", Style::getAttribute(json::theme::background_subtle));
	} else {
		style.replace("&&Background&&", "transparent");
		style.replace("&&HoverBackground&&", "transparent");
	}
	w->setStyleSheet(style);
}

void StyleHelper::HoverToolTip(QWidget *w, QString info, QString objectName)
{
	if(!objectName.isEmpty())
		w->setObjectName(objectName);

	QLabel *label = new QLabel(info);
	QString style = QString(R"css(QLabel {
				font-weight: bold;
				color: &&content_default&&;
				 }
				)css");
	style.replace("&&content_default&&", Style::getAttribute(json::theme::content_default));
	label->style();

	style = QString(R"css(QWidget {
			 background-color: &&background_primary&&;
			 }
				)css");
	style.replace("&&background_primary&&", Style::getAttribute(json::theme::background_primary));
	w->setStyleSheet(style);

	QHBoxLayout *layout = new QHBoxLayout(w);
	layout->addWidget(label);
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

void StyleHelper::BrowseButton(QPushButton *btn, QString objectName)
{
	if(!objectName.isEmpty()) {
		btn->setObjectName(objectName);
	}
	int btnSize = Style::getDimension(json::global::unit_2_5);
	int iconSize = Style::getDimension(json::global::unit_2);
	QString iconPath(":/gui/icons/choose_file.svg");
	btn->setIcon(Style::getPixmap(iconPath, Style::getColor(json::theme::content_inverse)));
	btn->setCheckable(false);
	btn->setFixedSize(btnSize, btnSize);
	btn->setIconSize(QSize(iconSize, iconSize));
	Style::setStyle(btn, style::properties::button::basicButton);
}

void StyleHelper::VerticalLine(QFrame *line, QString objectName)
{
	if(!objectName.isEmpty()) {
		line->setObjectName(objectName);
	}
	line->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	line->setFrameShape(QFrame::VLine);
	line->setFrameShadow(QFrame::Plain);
	line->setFixedWidth(1);

	Style::setStyle(line, style::properties::frame::line);
}

void StyleHelper::ColoredInteractiveLabel(InteractiveLabel *w, int index, QString objectName)
{
	if(!objectName.isEmpty()) {
		w->setObjectName(objectName);
	}

	QString style = QString(R"css(
				*{
					border-color: &&labelColor&&;
					color: &&labelColor&&;
				 }
				*:hover{
					border-color: &&hoverColor&&;
					color: &&hoverColor&&;
				 }
				)css");

	QColor color(getChannelColor(index));
	style.replace("&&labelColor&&", color.name());
	style.replace("&&hoverColor&&", color.lighter().name());

	w->setStyleSheet(w->styleSheet() + "\n" + style);
}

#include "moc_stylehelper.cpp"
