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

#include "genalyzerpanel.h"
#include <style.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QFont>
#include <QTextBrowser>

using namespace scopy;

GenalyzerChannelDisplay::GenalyzerChannelDisplay(const QString &channelName, QColor channelColor, QWidget *parent)
	: QTextBrowser(parent)
	, m_channelName(channelName)
	, m_channelColor(channelColor)
{
	setObjectName("GenalyzerChannelDisplay");

	// Disable editing and make it read-only
	setReadOnly(true);

	// Enable vertical scrolling but disable horizontal scrolling
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// Use monospace font for perfect alignment
	QFont font;
	font.setPixelSize(Style::getDimension(json::global::font_size));
	setFont(font);

	// Set channel color
	setChannelColor(channelColor);
}

void GenalyzerChannelDisplay::updateResults(size_t results_size, char **rkeys, double *rvalues)
{
	// Save current scroll position
	QScrollBar *vScrollBar = verticalScrollBar();
	int scrollPosition = vScrollBar->value();

	// Use setPlainText for maximum performance - no HTML parsing
	setPlainText(formatResultsText(results_size, rkeys, rvalues));

	// Restore scroll position
	vScrollBar->setValue(scrollPosition);
}

void GenalyzerChannelDisplay::setChannelColor(QColor color)
{
	m_channelColor = color;
	setStyleSheet("QTextBrowser { color: " + color.name() + "; border: none; background: transparent; }");
}

QString GenalyzerChannelDisplay::channelName() const { return m_channelName; }

QString GenalyzerChannelDisplay::formatResultsText(size_t results_size, char **rkeys, double *rvalues)
{
	if(results_size == 0) {
		return "No results";
	}

	// Use fast string building with pre-allocated size
	QString result;
	result.reserve(results_size * 30); // Pre-allocate to avoid reallocs

	const int keyWidth = 18;
	const int valueWidth = 12;

	// Build each key/value pair on a single row
	for(size_t i = 0; i < results_size; i++) {
		QString key = QString(rkeys[i]) + ":";
		QString value = QString::number(rvalues[i], 'f', 3);

		// Format: "key_name:         value"
		QString line = key.leftJustified(keyWidth) + value.rightJustified(valueWidth);
		result += line + '\n';
	}

	return result;
}

GenalyzerPanel::GenalyzerPanel(QWidget *parent)
	: QWidget(parent)
{
	m_panelLayout = new QVBoxLayout(this);
	setLayout(m_panelLayout);
	m_panelLayout->setMargin(6);
	m_panelLayout->setSpacing(6);
	m_panelLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	setFixedWidth(250); // Fixed width for right panel
	Style::setBackgroundColor(this, json::theme::background_subtle);
}

GenalyzerPanel::~GenalyzerPanel() { clear(); }

GenalyzerChannelDisplay *GenalyzerPanel::findOrCreateChannelDisplay(const QString &channelName, QColor channelColor)
{
	// Check if display for this channel already exists
	if(m_channelDisplays.contains(channelName)) {
		return m_channelDisplays[channelName];
	}

	// Create new display for this channel
	GenalyzerChannelDisplay *display = new GenalyzerChannelDisplay(channelName, channelColor, this);
	m_channelDisplays[channelName] = display;
	m_panelLayout->addWidget(display);

	return display;
}

void GenalyzerPanel::updateResults(const QString &channelName, QColor channelColor, size_t results_size, char **rkeys,
				   double *rvalues)
{
	// Find or create display for this channel (no flashing - reuses existing!)
	GenalyzerChannelDisplay *display = findOrCreateChannelDisplay(channelName, channelColor);

	// Update results in one operation - prevents flashing
	display->updateResults(results_size, rkeys, rvalues);
}

void GenalyzerPanel::clearChannel(const QString &channelName)
{
	if(m_channelDisplays.contains(channelName)) {
		GenalyzerChannelDisplay *display = m_channelDisplays.take(channelName);
		m_panelLayout->removeWidget(display);
		display->deleteLater();
	}
}

void GenalyzerPanel::clear()
{
	// Delete all channel displays
	for(auto it = m_channelDisplays.begin(); it != m_channelDisplays.end(); ++it) {
		it.value()->deleteLater();
	}
	m_channelDisplays.clear();
}

void GenalyzerPanel::setChannelVisible(const QString &channelName, bool visible)
{
	if(m_channelDisplays.contains(channelName)) {
		GenalyzerChannelDisplay *display = m_channelDisplays[channelName];
		display->setVisible(visible);
	}
}

#include "moc_genalyzerpanel.cpp"
