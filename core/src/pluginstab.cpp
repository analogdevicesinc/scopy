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

#include "pluginstab.h"

#include <QScrollArea>
#include <QVBoxLayout>
#include <cardwidget.h>
#include <menusectionwidget.h>
#include <pkgmanager.h>
#include <pluginrepository.h>
#include <style.h>
#include <stylehelper.h>

using namespace scopy;

PluginsTab::PluginsTab(QWidget *parent)
	: QWidget(parent)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setMargin(0);

	QWidget *w = new QWidget(this);
	QVBoxLayout *wLay = new QVBoxLayout(w);
	wLay->setMargin(10);

	MenuSectionCollapseWidget *pluginsSection = new MenuSectionCollapseWidget(
		"Plugins", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, w);
	pluginsSection->contentLayout()->setSpacing(10);
	pluginsSection->collapseSection()->header()->setCheckable(false);
	pluginsSection->menuSection()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_pluginsGrid = new GridWidget(pluginsSection);
	// The right margin is used to avoid the scroll bar overlapping with the plugin cards.
	QMargins rightMargin(0, 0, 2, 0);
	m_pluginsGrid->layout()->setContentsMargins(rightMargin);
	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setWidget(m_pluginsGrid);
	scrollArea->setWidgetResizable(true);
	addCards();
	pluginsSection->contentLayout()->addWidget(scrollArea);

	wLay->addWidget(pluginsSection);
	lay->addWidget(w);

	Style::setBackgroundColor(w, json::theme::background_subtle);
}

PluginsTab::~PluginsTab() {}

void PluginsTab::addCards()
{
	const QList<PluginInfo> plugins = PluginRepository::getPluginsInfo();
	for(const PluginInfo &p : plugins) {
		QString pkgName = PkgManager::reverseSearch(p.filePath()).baseName().toUpper();
		QString id = p.name() + "_" + pkgName;
		QString subtitle = (pkgName.isEmpty()) ? pkgName : "from " + pkgName;
		CardWidget *pluginCard = createPluginCard(id, p.name(), subtitle, p.description());
		QLabel *label;
		if(p.isLoaded()) {
			label = createStatusLabel("Loaded", "success");
		} else {
			label = createStatusLabel("Unloaded", "error");
		}
		pluginCard->layout()->addWidget(label);
		Style::setStyle(pluginCard, style::properties::widget::cardWidget, false);
		m_pluginsGrid->addWidget(pluginCard, pluginCard->id());
	}
}

QLabel *PluginsTab::createStatusLabel(const QString &text, const QString &status)
{
	QLabel *label = new QLabel(text);
	label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	Style::setStyle(label, style::properties::label::statusLabel, status);
	return label;
}

CardWidget *PluginsTab::createPluginCard(const QString &id, const QString &title, const QString &subtitle,
					 const QString &description, const QString &version)
{
	CardWidget *pluginCard = new CardWidget(m_pluginsGrid);
	pluginCard->setId(id);
	pluginCard->title()->setText(title);
	pluginCard->subtitle()->setText(subtitle);
	pluginCard->description()->setText(description);
	pluginCard->versCb()->insertItem(0, version);

	return pluginCard;
}

#include "moc_pluginstab.cpp"
