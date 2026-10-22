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

#include "plotinfo.h"

#include <QFontMetrics>
#include <QLabel>
#include <pluginbase/preferences.h>
#include <style.h>
#include <stylehelper.h>

using namespace scopy;

PlotInfo::PlotInfo(QWidget *parent)
	: QWidget(parent)
	, m_parent(parent)
	, m_margin(6)
	, m_spacing(6)
	, m_leftInfo(new QWidget())
	, m_rightInfo(new QWidget())
	, m_bottomLeftInfo(new QWidget())
	, m_bottomRightInfo(new QWidget())
{
	initLayouts();

	connect(Preferences::GetInstance(), &Preferences::preferenceChanged, this,
		[this](QString preference, QVariant) {
			if(preference == "plot_labels_inside") {
				applyMargins();
			}
		});
}

PlotInfo::~PlotInfo() {}

void PlotInfo::addCustomInfo(QWidget *info, InfoPosition hpos, InfoPosition vpos)
{
	QVBoxLayout *targetLayout = nullptr;
	QWidget *targetParent = nullptr;
	bool alignRight = (hpos == IP_RIGHT);

	if(vpos == IP_BOTTOM) {
		if(hpos == IP_LEFT) {
			targetLayout = m_bottomLeftLayout;
			targetParent = m_bottomLeftInfo;
		} else {
			targetLayout = m_bottomRightLayout;
			targetParent = m_bottomRightInfo;
		}
		targetLayout->insertWidget(0, info, 0, alignRight ? Qt::AlignRight : Qt::Alignment());
	} else {
		if(hpos == IP_LEFT) {
			targetLayout = m_leftLayout;
			targetParent = m_leftInfo;
		} else {
			targetLayout = m_rightLayout;
			targetParent = m_rightInfo;
		}
		targetLayout->addWidget(info, 0, alignRight ? Qt::AlignRight : Qt::Alignment());
	}

	info->setParent(targetParent);
	setMouseTransparent(info);

	if(alignRight) {
		QLabel *labelInfo = dynamic_cast<QLabel *>(info);
		if(labelInfo) {
			labelInfo->setAlignment(Qt::AlignRight);
		}
	}

	info->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

QLabel *PlotInfo::addLabelInfo(InfoPosition hpos, InfoPosition vpos)
{
	QLabel *label = new QLabel();
	addCustomInfo(label, hpos, vpos);

	return label;
}

void PlotInfo::removeInfo(uint index, InfoPosition pos)
{
	QWidget *widget = getInfo(index, pos);
	if(widget) {
		switch(pos) {
		case InfoPosition::IP_LEFT:
			m_leftLayout->removeWidget(widget);
			break;

		case InfoPosition::IP_RIGHT:
		default:
			m_rightLayout->removeWidget(widget);
			break;
		}
		widget->setParent(nullptr);
	}
}

QWidget *PlotInfo::getInfo(uint index, InfoPosition pos)
{
	switch(pos) {
	case InfoPosition::IP_LEFT:
		if(index >= m_leftLayout->count())
			return nullptr;
		return m_leftLayout->itemAt(index)->widget();

	case InfoPosition::IP_RIGHT:
	default:
		if(index >= m_rightLayout->count())
			return nullptr;
		return m_rightLayout->itemAt(index)->widget();
	}

	return nullptr;
}

void PlotInfo::initLayouts()
{
	// top left info
	m_leftLayout = new QVBoxLayout(m_leftInfo);
	m_leftLayout->setSpacing(m_spacing);

	m_leftHover = new HoverWidget(m_leftInfo, m_parent, m_parent);
	m_leftHover->setAnchorPos(HoverPosition::HP_TOPLEFT);
	m_leftHover->setContentPos(HoverPosition::HP_BOTTOMRIGHT);
	m_leftHover->show();
	setMouseTransparent(m_leftHover);

	// top right info
	m_rightLayout = new QVBoxLayout(m_rightInfo);
	m_rightLayout->setSpacing(m_spacing);

	m_rightHover = new HoverWidget(m_rightInfo, m_parent, m_parent);
	m_rightHover->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	m_rightHover->setContentPos(HoverPosition::HP_BOTTOMLEFT);
	m_rightHover->show();
	setMouseTransparent(m_rightHover);

	// bottom left info
	m_bottomLeftLayout = new QVBoxLayout(m_bottomLeftInfo);
	m_bottomLeftLayout->setSpacing(m_spacing);

	m_bottomLeftHover = new HoverWidget(m_bottomLeftInfo, m_parent, m_parent);
	m_bottomLeftHover->setAnchorPos(HoverPosition::HP_BOTTOMLEFT);
	m_bottomLeftHover->setContentPos(HoverPosition::HP_TOPRIGHT);
	m_bottomLeftHover->show();
	setMouseTransparent(m_bottomLeftHover);

	// bottom right info
	m_bottomRightLayout = new QVBoxLayout(m_bottomRightInfo);
	m_bottomRightLayout->setSpacing(m_spacing);

	m_bottomRightHover = new HoverWidget(m_bottomRightInfo, m_parent, m_parent);
	m_bottomRightHover->setAnchorPos(HoverPosition::HP_BOTTOMRIGHT);
	m_bottomRightHover->setContentPos(HoverPosition::HP_TOPLEFT);
	m_bottomRightHover->show();
	setMouseTransparent(m_bottomRightHover);

	applyMargins();
}

// Qt's hit test recurses into children before consulting the attribute, so the whole subtree has to
// carry it — a transparent label inside an opaque container is still unreachable. HoverWidget also
// overrides mousePressEvent without ignore(), so it consumes presses even when it is not draggable;
// being unhittable is what keeps the events off it and on the canvas, where PlotNavigator listens.
void PlotInfo::setMouseTransparent(QWidget *w)
{
	w->setAttribute(Qt::WA_TransparentForMouseEvents);
	for(QObject *child : w->children()) {
		if(QWidget *childWidget = qobject_cast<QWidget *>(child)) {
			setMouseTransparent(childWidget);
		}
	}
}

void PlotInfo::applyMargins()
{
	// With the axis labels drawn inside the canvas the corner groups would sit on top of them, so
	// back off by a label's worth of room. Doubled horizontally on purpose: the real labels carry a
	// unit suffix and a sign, so a placeholder of digits alone under-measures them. Vertically one
	// line is exact — the label row is one line tall.
	const bool inside = Preferences::get("plot_labels_inside").toBool();
	const QFontMetrics fm(font());
	const int padX = inside ? fm.horizontalAdvance("-000.00") * 2 : 0;
	const int padY = inside ? fm.height() : 0;

	for(QVBoxLayout *layout : {m_leftLayout, m_rightLayout, m_bottomLeftLayout, m_bottomRightLayout}) {
		layout->setContentsMargins(m_margin + padX, m_margin + padY, m_margin + padX, m_margin + padY);
	}
}

#include "moc_plotinfo.cpp"
