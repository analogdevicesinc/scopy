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

#ifndef SCOPYCODEEDITOR_H
#define SCOPYCODEEDITOR_H

#include <QPlainTextEdit>
#include "scopy-core_export.h"

namespace scopy {

class SCOPY_CORE_EXPORT ScopyCodeEditor : public QPlainTextEdit
{
	Q_OBJECT
public:
	explicit ScopyCodeEditor(QWidget *parent = nullptr);

	void lineNumberAreaPaintEvent(QPaintEvent *event);
	int lineNumberAreaWidth();
	void highlightLine(int lineNumber);

protected:
	void resizeEvent(QResizeEvent *event) override;

private slots:
	void updateLineNumberAreaWidth(int newBlockCount);
	void highlightCurrentLine();
	void updateLineNumberArea(const QRect &rect, int dy);

private:
	QWidget *lineNumberArea;
};

class LineNumberArea : public QWidget
{
	Q_OBJECT

public:
	LineNumberArea(ScopyCodeEditor *editor)
		: QWidget(editor)
		, codeEditor(editor)
	{}

	QSize sizeHint() const override { return QSize(codeEditor->lineNumberAreaWidth(), 0); }

protected:
	void paintEvent(QPaintEvent *event) override { codeEditor->lineNumberAreaPaintEvent(event); }

private:
	ScopyCodeEditor *codeEditor;
};

} // namespace scopy
#endif // SCOPYCODEEDITOR_H
