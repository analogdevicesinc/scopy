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
 */

#include "datastrategy/filedemodatastrategy.h"
#include <utility>
#include <QFile>

using namespace scopy;

FileDemoDataStrategy::FileDemoDataStrategy(IIOWidgetFactoryRecipe recipe, QWidget *parent)
	: QWidget(parent)
{
	m_recipe = recipe;
}

void FileDemoDataStrategy::save(QString data)
{
	QFile saveFile("/home/andrei-fabian/adi/newscopy/the_office.txt");
	saveFile.open(QIODevice::WriteOnly);
	data += "\n";
	saveFile.write(data.toLatin1());
	saveFile.close();
}

void FileDemoDataStrategy::requestData()
{
	Q_EMIT sendData("andrei", "adi alexandra andrei#1 andrei#2 andrei#3 cristi ionut bogdan");
}

#include "moc_filedemodatastrategy.cpp"
