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

	m_catBtnGroup = new QButtonGroup(this);
	m_catBtnGroup->setExclusive(false);

	m_crtCatBtn = nullptr;

	connect(m_catBtnGroup, qOverload<QAbstractButton *>(&QButtonGroup::buttonClicked), this,
		&PkgGridWidget::handleCategoryClick);

	m_maxCol = Preferences::get("pkg_menu_columns").toInt();
	connect(Preferences::GetInstance(), &Preferences::preferenceChanged, this, &PkgGridWidget::colNumberChanged);
}

PkgGridWidget::~PkgGridWidget() {}

void PkgGridWidget::addPkg(PkgItemWidget *pkgItem)
{
	int gridSize = m_pkgMap.size();
	m_layout->addWidget(pkgItem, gridSize / m_maxCol, gridSize % m_maxCol);
	fillCatBtnGroup(pkgItem->categoryBtns());
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

void PkgGridWidget::searchPkg(const QString &field, const QStringList &values)
{
	// Hide all the packages
	hideAll();
	// Reorganize the widgets within the layout
	int counter = 0;
	for(auto it = m_pkgMap.begin(); it != m_pkgMap.end(); ++it) {
		QVariantMap pkgMetadata = it.value()->metadata();
		if(!pkgMetadata.contains(field) || values.isEmpty()) {
			showPkg(it.value(), counter++);
			continue;
		}

		QVariant var = pkgMetadata[field];
		QStringList pkgValues =
			(var.type() == QVariant::String) ? QStringList{var.toString()} : var.toStringList();

		bool matchFound = false;
		for(const QString &pkgVal : pkgValues) {
			matchFound = isSearchMatch(pkgVal, values);
			if(matchFound) {
				showPkg(it.value(), counter++);
				break;
			}
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

void PkgGridWidget::handleCategoryClick(QAbstractButton *abstract)
{
	QPushButton *btn = dynamic_cast<QPushButton *>(abstract);
	if(m_crtCatBtn == btn) {
		btn->setChecked(false);
		m_crtCatBtn = nullptr;
	} else {
		if(m_crtCatBtn) {
			m_crtCatBtn->setChecked(false);
		}
		m_crtCatBtn = dynamic_cast<QPushButton *>(btn);
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

void PkgGridWidget::fillCatBtnGroup(const QList<QPushButton *> btns)
{
	for(QPushButton *btn : btns) {
		m_catBtnGroup->addButton(btn);
	}
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
