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

#include "analysismenu.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>

using namespace scopy::extprocplugin;

AnalysisMenu::AnalysisMenu(QWidget *parent)
	: QWidget(parent)
{
	m_factory = new MenuItemFactory(this);
	setupUI();
	connect(m_factory, &MenuItemFactory::itemParamChanged, this, &AnalysisMenu::updateAnalysisConfig);
}

AnalysisMenu::~AnalysisMenu() {}

void AnalysisMenu::setupUI()
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	layout->setMargin(0);

	m_w = new QWidget(this);
	m_w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_w->setLayout(new QVBoxLayout);
	m_w->layout()->setMargin(0);
	layout->addWidget(m_w);

	// Apply button
	QPushButton *applyButton = new QPushButton("Apply");
	layout->addWidget(applyButton);

	connect(applyButton, &QPushButton::clicked, this, &AnalysisMenu::applyPressed);
}

QVariantMap AnalysisMenu::getAnalysisConfig() { return m_analysisConfig; }

void AnalysisMenu::createMenu(const QVariantMap &params)
{
	QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(m_w->layout());
	// Clear existing items
	while(QLayoutItem *item = layout->takeAt(0)) {
		if(item->widget()) {
			item->widget()->deleteLater();
		}
		delete item;
	}

	// Create new menu items
	for(auto it = params.begin(); it != params.end(); ++it) {
		QWidget *menuItem = m_factory->createWidget(it.key(), it.value().toMap());
		if(menuItem) {
			layout->addWidget(menuItem);
		}
	}
}

void AnalysisMenu::updateAnalysisConfig(const QString &field, const QVariant &value)
{
	m_analysisConfig[field] = value;
}
