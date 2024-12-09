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

#ifndef SCOPYCONSOLEEDIT_H
#define SCOPYCONSOLEEDIT_H

#include "scopy-core_export.h"
#include <QPlainTextEdit>
#include <QWidget>

namespace scopy {
class SCOPY_CORE_EXPORT ScopyConsoleEdit : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit ScopyConsoleEdit(QWidget *parent = nullptr);

	QString prompt;

signals:
	void lineEntered(const QString &line);

protected:
	void keyPressEvent(QKeyEvent *e) override;
};
} // namespace scopy
#endif // SCOPYCONSOLEEDIT_H
