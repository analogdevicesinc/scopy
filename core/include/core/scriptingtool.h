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
#include <tooltemplate.h>
#include "scopy-core_export.h"
#include "scopycodeeditor.h"
#include <toolbuttons.h>

namespace scopy {

// ConsoleEdit: QPlainTextEdit that allows direct input and processes Enter key
class SCOPY_CORE_EXPORT ConsoleEdit : public QPlainTextEdit {
	Q_OBJECT
public:
	explicit ConsoleEdit(QWidget *parent = nullptr) : QPlainTextEdit(parent) {
		setUndoRedoEnabled(false);
		setTabChangesFocus(true);
		setLineWrapMode(QPlainTextEdit::NoWrap);
		prompt = ">>> ";
		appendPlainText(prompt);
		moveCursor(QTextCursor::End);
	}
	QString prompt;

signals:
	void lineEntered(const QString &line);

protected:
	void keyPressEvent(QKeyEvent *e) override {
		if (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter) {
			QTextCursor cursor = textCursor();
			cursor.movePosition(QTextCursor::End);
			setTextCursor(cursor);
			QString text = toPlainText();
			int lastPrompt = text.lastIndexOf(prompt);
			if (lastPrompt != -1) {
				QString line = text.mid(lastPrompt + prompt.length()).split('\n').first();
				emit lineEntered(line);
				if (line.trimmed() == "clear") {
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
};

class SCOPY_CORE_EXPORT ScriptingTool : public QWidget
{
	Q_OBJECT
public:
	explicit ScriptingTool(QWidget *parent = nullptr);

signals:

private:
	ToolTemplate *m_tool;
	ScopyCodeEditor *m_codeEditor;
	ConsoleEdit *m_console;
	RunBtn *m_runBtn;

	void loadFile();
	void saveToFile();
	void compileCode(QString code);

	//debug
	QPushButton *m_debugBtn;
	QPushButton *m_debugStepBtn;
	QStringList m_debugLines;
	bool debugMode = false;
	int debugLine = 0;

	void debugClicked();
	void debugStepClicked();
};

} // namespace scopy
#endif // SCRIPTINGTOOL_H
