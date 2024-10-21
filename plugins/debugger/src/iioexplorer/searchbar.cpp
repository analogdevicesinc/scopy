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

#include "searchbar.h"
#include "style_properties.h"
#include <QHBoxLayout>
#include <style.h>

using namespace scopy::debugger;

SearchBar::SearchBar(QSet<QString> options, QWidget *parent)
	: QWidget(parent)
	, m_lineEdit(new QLineEdit(this))
	, m_label(new QLabel("Filter", this))
	, m_completer(new QCompleter(options.values(), this))
{
	Style::setStyle(m_lineEdit, style::properties::searchBar);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setFilterMode(Qt::MatchContains);
	m_lineEdit->setCompleter(m_completer);

	setLayout(new QVBoxLayout(this));
	layout()->addWidget(m_label);
	layout()->addWidget(m_lineEdit);
	layout()->setMargin(0);
}

QLineEdit *SearchBar::getLineEdit() { return m_lineEdit; }

#include "moc_searchbar.cpp"
