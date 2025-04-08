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
	: QWidget(parent)
	, m_maxCol(1)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setMargin(0);

	QWidget *pkgsW = new QWidget(this);
	m_layout = new QGridLayout(pkgsW);
	m_layout->setMargin(0);

	lay->addWidget(pkgsW);
	lay->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	m_maxCol = Preferences::get("pkg_menu_columns").toInt();
	connect(Preferences::GetInstance(), &Preferences::preferenceChanged, this, &PkgGridWidget::colNumberChanged);
}

PkgGridWidget::~PkgGridWidget() {}

void PkgGridWidget::addPkg(PkgItemWidget *pkgItem)
{
	int gridSize = m_pkgMap.size();
	m_layout->addWidget(pkgItem, gridSize / m_maxCol, gridSize % m_maxCol);
	m_pkgMap.insert(pkgItem->id(), pkgItem);
}

void PkgGridWidget::removePkg(const QString &pkgName)
{
	if(!m_pkgMap.contains(pkgName)) {
		return;
	}
	QWidget *w = m_pkgMap.take(pkgName);
	m_layout->removeWidget(w);
	w->deleteLater();
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

void PkgGridWidget::searchPkg(const QStringList &fields, const QStringList &options)
{
	// Hide all the packages
	hideAll();
	// Reorganize the widgets within the layout
	int counter = 0;
	for(auto it = m_pkgMap.begin(); it != m_pkgMap.end(); ++it) {
		QVariantMap pkgMetadata = it.value()->metadata();
		bool matchFound = false;
		for(const QString &field : fields) {
			if(isFieldMatch(pkgMetadata, field, options)) {
				matchFound = true;
				break;
			}
		}
		if(matchFound || options.isEmpty()) {
			showPkg(it.value(), counter++);
		}
	}

	Q_ASSERT(counter <= m_pkgMap.size());
}

void PkgGridWidget::colNumberChanged(QString pref, QVariant val)
{
	int col = val.toInt();
	if(pref == "pkg_menu_columns") {
		m_maxCol = col;
		rebuildLayout();
	}
}

void PkgGridWidget::hideAll()
{
	for(auto it = m_pkgMap.begin(); it != m_pkgMap.end(); ++it) {
		it.value()->hide();
	}
}

void PkgGridWidget::rebuildLayout()
{
	hideAll();
	int counter = 0;
	for(auto it = m_pkgMap.begin(); it != m_pkgMap.end(); ++it) {
		showPkg(it.value(), counter++);
	}
	Q_ASSERT(counter <= m_pkgMap.size());
}

void PkgGridWidget::showPkg(QWidget *pkg, int index)
{
	m_layout->addWidget(pkg, index / m_maxCol, index % m_maxCol);
	pkg->show();
}

bool PkgGridWidget::isSearchMatch(const QString &pkgVal, const QStringList &searchingValues)
{
	bool matchFound = false;
	for(const QString &searchingVal : searchingValues) {
		if(pkgVal.contains(searchingVal, Qt::CaseInsensitive)) {
			matchFound = true;
			break;
		}
	}
	return matchFound;
}

void PkgGridWidget::updatePkgsStyle(bool property)
{
	for(auto it = m_pkgMap.begin(); it != m_pkgMap.end(); ++it) {
		Style::setStyle(it.value(), style::properties::widget::pkgItem, property);
	}
}

#include "moc_pkggridwidget.cpp"
