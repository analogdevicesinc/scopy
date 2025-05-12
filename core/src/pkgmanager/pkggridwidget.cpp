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

#include "pkggridwidget.h"

#include <pluginbase/preferences.h>

#include <style.h>
#include <stylehelper.h>

using namespace scopy;

PkgGridWidget::PkgGridWidget(QWidget *parent)
	: GridWidget(parent)
{
	m_maxCol = Preferences::get("pkg_menu_columns").toInt();
	connect(Preferences::GetInstance(), &Preferences::preferenceChanged, this, [this](QString pref, QVariant val) {
		if(pref == "pkg_menu_columns") {
			setColumnCount(val.toInt());
		}
	});
}

PkgGridWidget::~PkgGridWidget() {}

void PkgGridWidget::searchPkg(const QStringList &fields, const QStringList &options)
{
	hideAll();
	int counter = 0;
	for(auto it = m_widgetMap.begin(); it != m_widgetMap.end(); ++it) {
		PkgCard *pkgCard = dynamic_cast<PkgCard *>(it.value());
		if(!pkgCard) {
			continue;
		}
		QVariantMap pkgMetadata = pkgCard->metadata();
		bool matchFound = false;
		for(const QString &field : fields) {
			if(isFieldMatch(pkgMetadata, field, options)) {
				matchFound = true;
				break;
			}
		}
		if(matchFound || options.isEmpty()) {
			m_layout->addWidget(it.value(), counter / m_maxCol, counter % m_maxCol);
			it.value()->show();
			counter++;
		}
	}
}

void PkgGridWidget::updatePkgsStyle(bool property)
{
	for(auto it = m_widgetMap.begin(); it != m_widgetMap.end(); ++it) {
		Style::setStyle(it.value(), style::properties::widget::cardWidget, property);
	}
}

bool PkgGridWidget::isFieldMatch(const QVariantMap &pkgMetadata, const QString &field, const QStringList &options)
{
	if(!pkgMetadata.contains(field) || options.isEmpty()) {
		return false;
	}

	QVariant var = pkgMetadata[field];
	QStringList pkgValues = (var.type() == QVariant::String) ? QStringList{var.toString()} : var.toStringList();

	for(const QString &pkgVal : pkgValues) {
		if(isSearchMatch(pkgVal, options)) {
			return true;
		}
	}
	return false;
}

bool PkgGridWidget::isSearchMatch(const QString &pkgVal, const QStringList &searchingValues)
{
	for(const QString &searchingVal : searchingValues) {
		if(pkgVal.contains(searchingVal, Qt::CaseInsensitive)) {
			return true;
		}
	}
	return false;
}
