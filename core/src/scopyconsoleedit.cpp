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

#include "scopyconsoleedit.h"

using namespace scopy;

ScopyConsoleEdit::ScopyConsoleEdit(QWidget *parent)
	: QPlainTextEdit(parent)
{
	setUndoRedoEnabled(false);
	setTabChangesFocus(true);
	setLineWrapMode(QPlainTextEdit::NoWrap);
	prompt = ">>> ";
	appendPlainText(prompt);
	moveCursor(QTextCursor::End);
}

void ScopyConsoleEdit::keyPressEvent(QKeyEvent *e)
{
	if(e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
		QTextCursor cursor = textCursor();
		cursor.movePosition(QTextCursor::End);
		setTextCursor(cursor);
		QString text = toPlainText();
		int lastPrompt = text.lastIndexOf(prompt);
		if(lastPrompt != -1) {
			QString line = text.mid(lastPrompt + prompt.length()).split('\n').first();
			emit lineEntered(line);
			if(line.trimmed() == "clear") {
				clear();
				appendPlainText(prompt);
			} else {
				appendPlainText("");
				appendPlainText(prompt);
			}
			moveCursor(QTextCursor::End);
		}
	} else {
		QPlainTextEdit::keyPressEvent(e);
	}
}

#include "moc_scopyconsoleedit.cpp"
