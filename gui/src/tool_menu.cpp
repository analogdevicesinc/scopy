/*
 * Copyright (c) 2019 Analog Devices Inc.
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

#include <QDebug>

#include <scopy/gui/tool_menu.hpp>

using namespace scopy::gui;

ToolMenu::ToolMenu(Preferences* preferences, QWidget* parent)
	: BaseMenu(parent)
	, m_currentHwName("")
	, m_buttonGroup(nullptr)
	, m_preferences(preferences)
{
	m_availableTools = QStringList() << tr("Oscilloscope") << tr("Spectrum Analyzer") << tr("Network Analyzer")
					 << tr("Signal Generator") << tr("Logic Analyzer") << tr("Pattern Generator")
					 << tr("Digital IO") << tr("Voltmeter") << tr("Power Supply") << tr("Debugger")
					 << tr("Calibration");

	m_availableIcons = QStringList() << "oscilloscope"
					 << "spectrum_analyzer"
					 << "network_analyzer"
					 << "signal_generator"
					 << "logic_analyzer"
					 << "pattern_generator"
					 << "io"
					 << "voltmeter"
					 << "power_supply"
					 << "debugger"
					 << "calibration";
	_loadState();

	m_buttonGroup = new QButtonGroup(this);
	_buildAllAvailableTools();

	connect(this, &ToolMenu::itemMovedFromTo, this, &ToolMenu::_updateToolList);
}

ToolMenu::~ToolMenu() { _saveState(); }

void ToolMenu::loadToolsFromFilter(Filter* filter)
{

	if (!filter) {
		for (int i = 0; i < m_tools.size(); ++i) {
			removeMenuItem(m_tools[i].first);
		}
		m_currentHwName = "";
		return;
	}

	if (m_currentHwName == filter->hwName()) {
		return;
	}

	m_currentHwName = filter->hwName();

	m_compatibleTools.clear();

	QVector<BaseMenuItem*> notCompatibleTools;
	QVector<int> compatiblePositions;
	for (int i = 0; i < m_tools.size(); ++i) {
		if (filter->compatible(m_tools[i].second)) {
			m_compatibleTools.push_back(m_tools[i].first);
			compatiblePositions.push_back(i);
		} else {
			notCompatibleTools.push_back(m_tools[i].first);
		}
	}
	if (notCompatibleTools.size() && m_items) {
		removeMenuItem(notCompatibleTools);
	} else {
		insertMenuItem(m_compatibleTools, compatiblePositions);
	}
}

ToolMenuItem* ToolMenu::getToolMenuItemFor(enum tool tool)
{
	for (int i = 0; i < m_tools.size(); ++i) {
		if (m_tools[i].second == tool) {
			return static_cast<ToolMenuItem*>(m_tools[i].first);
		}
	}
	return nullptr;
}

QButtonGroup* ToolMenu::getButtonGroup() { return m_buttonGroup; }

void ToolMenu::hideMenuText(bool hidden)
{
	for (int i = 0; i < m_tools.size(); ++i) {
		static_cast<ToolMenuItem*>(m_tools[i].first)->hideText(hidden);
	}
}

void ToolMenu::_updateToolList(short from, short to)
{
	if (m_items == m_tools.size()) {
		auto toMove = m_tools[from];
		m_tools.remove(from);
		m_tools.insert(to, toMove);
	}
}

void ToolMenu::_buildAllAvailableTools()
{
	if (m_positions.empty()) {
		for (int i = 0; i < m_availableTools.size(); ++i) {
			m_positions.push_back(i);
		}
	} else {
		while ((m_positions.size() < m_availableTools.size())) {
			m_positions.push_back(m_positions.size());
		}
	}

	for (int i = 0; i < m_availableTools.size(); ++i) {
		ToolMenuItem* item =
			new ToolMenuItem(m_availableTools[m_positions[i]], m_availableIcons[m_positions[i]], this);
		connect(item->getToolBtn(), &QPushButton::clicked, [=]() {
			if (item->isDetached()) {
				item->detach();
			} else {
				Q_EMIT toolSelected(static_cast<tool>(m_positions[i]));
			}
		});
		connect(item, &ToolMenuItem::toggleButtonGroup, [=](bool detached) {
			if (detached) {
				m_buttonGroup->removeButton(item->getToolBtn());
			} else {
				m_buttonGroup->addButton(item->getToolBtn());
			}
		});
		connect(item, &ToolMenuItem::enableInfoWidget, this, &ToolMenu::enableInfoWidget);
		m_buttonGroup->addButton(item->getToolBtn());
		m_tools.push_back(QPair<BaseMenuItem*, tool>(item, static_cast<tool>(m_positions[i])));
		m_tools[i].first->setVisible(false);
	}
}

void ToolMenu::_saveState()
{
	QSettings settings;

	settings.beginWriteArray("toolMenu/pos");
	for (int i = 0; i < m_tools.size(); ++i) {
		settings.setArrayIndex(i);
		settings.setValue("idx", QVariant(m_tools[i].second));
	}
	settings.endArray();
}

void ToolMenu::_loadState()
{
	QSettings settings;

	int n = settings.beginReadArray("toolMenu/pos");
	for (int i = 0; i < n; ++i) {
		settings.setArrayIndex(i);
		m_positions.push_back(settings.value("idx").value<int>());
	}
	settings.endArray();
}
