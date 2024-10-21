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

#include "pathtitle.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <gui/stylehelper.h>

// Currently there is no need for a custom split char, so it will be unchangeable
#define SPLIT_CHAR '/'

using namespace scopy;

PathTitle::PathTitle(QWidget *parent)
	: QWidget(parent)
{
	setupUi();
}

PathTitle::PathTitle(QString title, QWidget *parent)
	: QWidget(parent)
{
	setupUi();
	setTitle(title);
}

void PathTitle::setTitle(QString title)
{
	m_titlePath = title;

	// Delete all children
	while(QWidget *child = findChild<QWidget *>()) {
		delete child;
	}

	// Add new children
	QStringList segmentStrings = title.split(SPLIT_CHAR, Qt::SkipEmptyParts);
	QString auxPath;

	for(int i = 0; i < segmentStrings.size(); ++i) {
		QPushButton *btn = new QPushButton(segmentStrings[i], this);
		StyleHelper::GrayButton(btn, "SegmentButton");
		auxPath.append(segmentStrings[i]);
		connect(btn, &QPushButton::clicked, this, [this, auxPath]() { Q_EMIT pathSelected(auxPath); });
		layout()->addWidget(btn);

		if(i < segmentStrings.size() - 1) {
			auxPath.append("/");
		}
	}
}

QString PathTitle::title() const { return m_titlePath; }

void PathTitle::setupUi()
{
	setObjectName("PathTitle");
	setLayout(new QHBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);
}
