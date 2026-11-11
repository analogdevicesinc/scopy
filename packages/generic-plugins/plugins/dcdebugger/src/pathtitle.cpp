/*
 * Copyright (c) 2026 Analog Devices Inc.
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
#include <QPushButton>

#include <style.h>

using namespace scopy;
using namespace scopy::dcdebugger;

PathTitle::PathTitle(const QList<Segment> &segments, QWidget *parent)
	: QWidget(parent)
{
	auto *lay = new QHBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(4);
	for(const Segment &s : segments) {
		auto *btn = new QPushButton(s.first, this);
		btn->setToolTip(s.first);
		Style::setStyle(btn, style::properties::button::borderButton);
		btn->setFixedHeight(buttonHeight());
		QPointer<QObject> node = s.second;
		connect(btn, &QPushButton::clicked, this, [this, node]() {
			if(node) {
				Q_EMIT pathSelected(node);
			}
		});
		lay->addWidget(btn);
	}
	lay->addStretch();
}

int PathTitle::buttonHeight() { return Style::getDimension(json::global::unit_2); }

#include "moc_pathtitle.cpp"
