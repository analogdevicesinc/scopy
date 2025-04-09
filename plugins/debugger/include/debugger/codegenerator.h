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

#ifndef CODEGENERATOR_H
#define CODEGENERATOR_H

#include <QObject>
#include <QList>
#include "iiostandarditem.h"

class CodeGenerator : public QObject
{
	Q_OBJECT
public:
	struct CodeGeneratorRecipe
	{
		QString uri;
		QString deviceName;
		QString channelName;
		bool channelIsInput;
		QString attributeName;
	};

	static QString generateCode(QList<CodeGeneratorRecipe> recipes);
	static CodeGenerator::CodeGeneratorRecipe convertToCodeGeneratorRecipe(scopy::debugger::IIOStandardItem *item,
									       QString uri);
	static QString var(QString name);
};

#endif // CODEGENERATOR_H
