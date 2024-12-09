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

#ifndef SCRIPTINGTOOL_H
#define SCRIPTINGTOOL_H

#include <QPlainTextEdit>
#include <QWidget>
#include "scopy-scripting_export.h"
#include "scopycodeeditor.h"

namespace scopy {
namespace scripting {

class SCOPY_SCRIPTING_EXPORT ScriptingTool : public QWidget
{
	Q_OBJECT
public:
	explicit ScriptingTool(QWidget *parent = nullptr);

signals:

private:
	QString fileName = "";
	ScopyCodeEditor *codeEditor;
	QPlainTextEdit *codeOutput;

	void loadFile();
	void saveToFile();
	void compileCode();
};

} // namespace scripting
} // namespace scopy
#endif // SCRIPTINGTOOL_H
