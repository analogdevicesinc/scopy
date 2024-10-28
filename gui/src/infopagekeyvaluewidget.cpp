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

#include "infopagekeyvaluewidget.h"

using namespace scopy;
InfoPageKeyValueWidget::InfoPageKeyValueWidget(QString key, QString value, QWidget *parent)
	:

	QWidget(parent)
{
	QHBoxLayout *lay = new QHBoxLayout(this);

	lay->setContentsMargins(0, 0, 0, 0);
	lay->setMargin(0);
	lay->setSpacing(0);

	keyWidget = new QLabel(key, this);
	valueWidget = new QLabel(value, this);
	lay->addWidget(keyWidget);
	lay->addWidget(valueWidget);
	lay->setStretch(0, 1);
	lay->setStretch(1, 3);
}

void InfoPageKeyValueWidget::updateValue(QString value) { valueWidget->setText(value); }
