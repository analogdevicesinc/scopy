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

#include "widgets/plotlegend.h"
#include "hoverwidget.h"
#include "plotchannel.h"
#include <QHBoxLayout>
#include <style.h>

using namespace scopy;

PlotLegend::PlotLegend(PlotWidget *plot, QWidget *parent)
	: QWidget(parent)
	, m_plot(plot)
	, m_visible(false)
{
	setContentsMargins(0, 0, 0, 0);
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

	m_legendPanel = new QWidget(m_plot);
	m_legendLayout = new QVBoxLayout(m_legendPanel);
	m_legendLayout->setContentsMargins(6, 6, 6, 6);
	m_legendLayout->setSpacing(4);

	m_hoverWidget = new HoverWidget(m_legendPanel, m_plot, m_plot);
	m_hoverWidget->setAnchorPos(HoverPosition::HP_BOTTOMRIGHT);
	m_hoverWidget->setContentPos(HoverPosition::HP_TOPLEFT);
	m_hoverWidget->setDraggable(true);
	m_hoverWidget->setRelative(true);
	m_hoverWidget->setVisible(false);

	connect(m_plot, &PlotWidget::addedChannel, this, &PlotLegend::addChannel);
	connect(m_plot, &PlotWidget::removedChannel, this, &PlotLegend::removeChannel);

	for(PlotChannel *ch : m_plot->getChannels()) {
		addChannel(ch);
	}
}

PlotLegend::~PlotLegend() {}

void PlotLegend::setVisible(bool visible)
{
	m_visible = visible;
	if(visible) {
		updateStyles();
	}
	m_hoverWidget->setVisible(visible);
	QWidget::setVisible(visible);
}

void PlotLegend::addChannel(PlotChannel *ch)
{
	if(m_entries.contains(ch)) {
		return;
	}

	QWidget *entry = new QWidget(m_legendPanel);
	Style::setBackgroundColor(entry, QString("transparent"), true);
	QHBoxLayout *entryLayout = new QHBoxLayout(entry);
	entryLayout->setContentsMargins(0, 0, 0, 0);
	entryLayout->setSpacing(6);

	QWidget *colorLine = new QWidget(entry);
	colorLine->setFixedSize(20, 3);

	QLabel *nameLabel = new QLabel(ch->name(), entry);

	entryLayout->addWidget(colorLine);
	entryLayout->addWidget(nameLabel);
	entryLayout->addStretch();

	m_legendLayout->addWidget(entry);
	m_entries.insert(ch, entry);
	m_labels.insert(ch, nameLabel);
	m_colorLines.insert(ch, colorLine);

	updateLineStyle(ch);

	connect(ch, &PlotChannel::enabledChanged, this, &PlotLegend::updateStyles);
	connect(ch, &PlotChannel::thicknessChanged, this, [this, ch]() { updateLineStyle(ch); });
	connect(ch, &PlotChannel::styleChanged, this, [this, ch]() { updateLineStyle(ch); });
}

void PlotLegend::removeChannel(PlotChannel *ch)
{
	if(!m_entries.contains(ch)) {
		return;
	}

	disconnect(ch, &PlotChannel::enabledChanged, this, &PlotLegend::updateStyles);
	disconnect(ch, &PlotChannel::thicknessChanged, this, nullptr);
	disconnect(ch, &PlotChannel::styleChanged, this, nullptr);

	QWidget *entry = m_entries.take(ch);
	m_labels.remove(ch);
	m_colorLines.remove(ch);
	m_legendLayout->removeWidget(entry);
	entry->deleteLater();
}

void PlotLegend::updateStyles()
{
	for(auto it = m_entries.begin(); it != m_entries.end(); ++it) {
		it.value()->setVisible(it.key()->isEnabled());
	}
}

void PlotLegend::updateLineStyle(PlotChannel *ch)
{
	if(!m_colorLines.contains(ch)) {
		return;
	}

	QWidget *colorLine = m_colorLines.value(ch);
	QColor penColor = ch->curve()->pen().color();
	QString color = QString("rgba(%1, %2, %3, %4)")
				.arg(penColor.red())
				.arg(penColor.green())
				.arg(penColor.blue())
				.arg(penColor.alpha());
	int thickness = ch->thickness();
	int style = ch->style();

	QString borderStyle;
	switch(style) {
	case PlotChannel::PCS_DOTS:
		borderStyle = QString("background-color: transparent; border: %1px dotted %2;").arg(thickness).arg(color);
		break;
	case PlotChannel::PCS_STICKS:
	case PlotChannel::PCS_STEPS:
		borderStyle = QString("background-color: transparent; border: %1px dashed %2;").arg(thickness).arg(color);
		break;
	case PlotChannel::PCS_LINES:
	case PlotChannel::PCS_SMOOTH:
	default:
		borderStyle = QString("background-color: %1;").arg(color);
		break;
	}

	colorLine->setFixedHeight(thickness);
	colorLine->setStyleSheet(borderStyle);
}

#include "moc_plotlegend.cpp"
