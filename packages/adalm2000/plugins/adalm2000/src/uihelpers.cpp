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
 */

#include "uihelpers.h"

#include <gui/style.h>

#include <QPushButton>
#include <QVBoxLayout>

using namespace scopy;

QString adalm2000::ui::channelName(int ch) { return QStringLiteral("CH%1").arg(ch + 1); }

QPushButton *adalm2000::ui::button(const QString &text, QWidget *parent)
{
	auto *b = new QPushButton(text, parent);
	Style::setStyle(b, style::properties::button::basicButton);
	return b;
}

gui::MenuSpinbox *adalm2000::ui::spin(const QString &name, double value, const QString &unit, double min, double max,
				      gui::MenuSpinbox::IncrementMode mode, QWidget *parent)
{
	auto *s = new gui::MenuSpinbox(name, value, unit, min, max, true, false, true, parent);
	s->setIncrementMode(mode);
	return s;
}

QWidget *adalm2000::ui::withStretch(const QList<QWidget *> &content)
{
	auto *w = new QWidget();
	auto *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(10);
	for(QWidget *c : content) {
		lay->addWidget(c);
	}
	lay->addStretch();
	return w;
}
