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

#pragma once

#include <gui/widgets/menuspinbox.h>

#include <QList>
#include <QString>

class QPushButton;
class QWidget;

namespace scopy::adalm2000::ui {

QString channelName(int ch);
QPushButton *button(const QString &text, QWidget *parent);
gui::MenuSpinbox *spin(const QString &name, double value, const QString &unit, double min, double max,
		       gui::MenuSpinbox::IncrementMode mode, QWidget *parent = nullptr);
// A menu page: the content on top, stretch below.
QWidget *withStretch(const QList<QWidget *> &content);

} // namespace scopy::adalm2000::ui
