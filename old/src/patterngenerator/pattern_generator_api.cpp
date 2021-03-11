/*
 * Copyright (c) 2020 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#include "pattern_generator_api.h"
#include "ui_pattern_generator.h"

#include "patterns/patterns.hpp"

#include <QCheckBox>

using namespace adiscope;
using namespace adiscope::logic;

QList<int> logic::PatternGenerator_API::getEnabledChannels() const
{
	QList<int> enabledChannels;
	for (int i = 0; i < 16; ++i) {
		QWidget *widgetInLayout = m_pattern->m_ui->channelEnumeratorLayout->itemAtPosition(i % 8,
							    i / 8)->widget();
		auto channelBox = dynamic_cast<QCheckBox *>(widgetInLayout);
		if (channelBox->isChecked()) {
			enabledChannels.push_back(i);
		}
	}
	return enabledChannels;
}

void logic::PatternGenerator_API::setEnabledChannels(const QList<int> &enabledChannels)
{
	for (const auto &channel : enabledChannels) {
		QWidget *widgetInLayout = m_pattern->m_ui->channelEnumeratorLayout->itemAtPosition(channel % 8,
							    channel / 8)->widget();
		auto channelBox = dynamic_cast<QCheckBox *>(widgetInLayout);
		channelBox->setChecked(true);
	}
}

QStringList logic::PatternGenerator_API::getChannelNames() const
{
	QStringList names;
	for (int i = 0; i < m_pattern->m_plotCurves.size(); ++i) {
		names.push_back(m_pattern->m_plotCurves[i]->getName());
	}
	return names;
}

void logic::PatternGenerator_API::setChannelNames(const QStringList &channelNames)
{
	for (int i = 0; i < channelNames.size(); ++i) {
		if (i >= m_pattern->m_plotCurves.size()) { break; }
		m_pattern->m_plotCurves[i]->setName(channelNames[i]);
	}
}

QList<double> logic::PatternGenerator_API::getChannelHeights() const
{
	QList<double> heights;
	for (int i = 0; i < m_pattern->m_plotCurves.size(); ++i) {
		heights.push_back(m_pattern->m_plotCurves[i]->getTraceHeight());
	}
	return heights;
}

void logic::PatternGenerator_API::setChannelHeights(const QList<double> &channelHeights)
{
	for (int i = 0; i < channelHeights.size(); ++i) {
		if (i >= m_pattern->m_plotCurves.size()) { break; }
		m_pattern->m_plotCurves[i]->setTraceHeight(channelHeights[i]);
	}
}

QList<double> logic::PatternGenerator_API::getChannelPosition() const
{
	QList<double> channelPosition;
	for (int i = 0; i < m_pattern->m_plotCurves.size(); ++i) {
		channelPosition.push_back(m_pattern->m_plotCurves[i]->getPixelOffset());
	}
	return channelPosition;
}

void logic::PatternGenerator_API::setChannelPosition(const QList<double> &channelPosition)
{
	for (int i = 0; i < channelPosition.size(); ++i) {
		if (i >= m_pattern->m_plotCurves.size()) { break; }
		m_pattern->m_plotCurves[i]->setPixelOffset(channelPosition[i]);
	}
}

QVector<QVector<int> > logic::PatternGenerator_API::getCurrentGroups() const
{
	QVector<QVector<int> > groups = m_pattern->m_plot.getAllGroups();
	for (auto &group : groups) {
		for (auto &ch : group) {
			if (ch >= 16) {
				group.removeOne(ch);
			}
		}
	}
	return groups;
}

void logic::PatternGenerator_API::setCurrentGroups(const QVector<QVector<int> > &groups)
{
	m_pattern->m_plot.setGroups(groups);
}

QVector<QPair<QVector<int>, QString> > logic::PatternGenerator_API::getEnabledPatterns() const
{
	QVector<QPair<QVector<int>, QString> > enabledPatterns;
	for (const QPair<QVector<int>, PatternUI *> &pattern : m_pattern->m_enabledPatterns) {
		enabledPatterns.push_back({pattern.first,
					Pattern_API::toString(pattern.second->get_pattern())});
	}

	return enabledPatterns;
}

void logic::PatternGenerator_API::setEnabledPatterns(const QVector<QPair<QVector<int>, QString> > &enabledPatterns)
{
	for (const QPair<QVector<int>, QString> &pattern : enabledPatterns) {
		m_pattern->patternSelected("", pattern.first.first(), pattern.second);
	}
}

QString logic::PatternGenerator_API::getNotes()
{
	return m_pattern->m_ui->instrumentNotes->getNotes();
}
void logic::PatternGenerator_API::setNotes(QString str)
{
	m_pattern->m_ui->instrumentNotes->setNotes(str);
}

