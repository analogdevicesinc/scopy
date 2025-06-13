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

#include "scriptingtool.h"
#include <QProcess>
#include <QPushButton>
#include <QBoxLayout>
#include <QFileDialog>
#include <QTextStream>
#include <QDebug>
#include <QJSValue>
#include <pluginbase/preferences.h>
#include <pluginbase/scopyjs.h>
#include <QDateTime>
#include <menuonoffswitch.h>
#include <style.h>
#include <QKeyEvent>
#include <QCoreApplication>
#include <QLineEdit>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_SCRIPTINGTOOL, "ScriptingTool")

ScriptingTool::ScriptingTool(QWidget *parent)
	: QWidget{parent}
{

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setContentsMargins(0, 0, 0, 0);
	this->setLayout(layout);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	m_tool->centralContainer()->layout()->setSpacing(10);

	layout->addWidget(m_tool);

	QPushButton *loadBtn = new QPushButton("Load", m_tool);
	QPushButton *saveBtn = new QPushButton("Save", m_tool);
	m_runBtn = new RunBtn(this);

	Style::setStyle(loadBtn, style::properties::button::basicButtonBig);
	Style::setStyle(saveBtn, style::properties::button::basicButtonBig);

	m_tool->addWidgetToTopContainerHelper(loadBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(saveBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);

	m_codeEditor = new ScopyCodeEditor(m_tool);
	Style::setStyle(m_codeEditor, style::properties::widget::basicComponent);
	Style::setStyle(m_codeEditor, style::properties::widget::border_interactive);

	connect(loadBtn, &QPushButton::clicked, this, [=]() { loadFile(); });
	connect(saveBtn, &QPushButton::clicked, this, [=]() { saveToFile(); });
	connect(m_runBtn, &QPushButton::clicked, this, [=]() { evaluateCode(m_codeEditor->toPlainText()); });

	m_console = new QPlainTextEdit(m_tool);
	m_console->setReadOnly(true);
	m_console->setUndoRedoEnabled(false);
	m_console->setTabChangesFocus(true);
	m_console->setLineWrapMode(QPlainTextEdit::NoWrap);

	Style::setStyle(m_console, style::properties::widget::basicComponent);
	Style::setStyle(m_console, style::properties::widget::border_interactive);

	// QLineEdit for user input with prompt
	QLineEdit *m_inputLine = new QLineEdit(m_tool);
	m_inputLine->setPlaceholderText(">>>");

	Style::setStyle(m_inputLine, style::properties::widget::basicComponent);
	Style::setStyle(m_inputLine, style::properties::widget::border_interactive);

	m_tool->addWidgetToCentralContainerHelper(m_codeEditor);
	m_tool->addWidgetToCentralContainerHelper(m_console);
	m_tool->addWidgetToCentralContainerHelper(m_inputLine);

	connect(m_inputLine, &QLineEdit::returnPressed, this, [this, m_inputLine]() {
		QString input = m_inputLine->text();
		if(!input.trimmed().isEmpty()) {
			// Show prompt and input in console
			m_console->appendPlainText(">>> " + input);
			evaluateCode(input);
		}
		m_inputLine->clear();
	});
}

void ScriptingTool::loadFile()
{
	// chose file
	bool useNativeDialogs = Preferences::get("general_use_native_dialogs").toBool();
	QString selectedFilter;
	QString fileName = QFileDialog::getOpenFileName(
		this, tr("Export"), "", tr("All Files(*)"), &selectedFilter,
		(useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));

	// display file content
	QFile file(fileName);

	if(!file.isOpen()) {
		file.open(QIODevice::ReadOnly);

		QTextStream in(&file);
		QString fileContent = "";
		while(!in.atEnd()) {
			QString line = in.readLine();
			fileContent += line + "\n";
		}
		m_codeEditor->appendPlainText(fileContent);
	} else {
		qDebug() << "File already opened! ";
		m_console->appendPlainText("File already opened! ");
	}

	if(file.isOpen())
		file.close();
}

void ScriptingTool::saveToFile()
{
	bool useNativeDialogs = Preferences::get("general_use_native_dialogs").toBool();
	QString selectedFilter;
	QString fileName = QFileDialog::getSaveFileName(
		this, tr("Export"), "", tr("All Files(*)"), &selectedFilter,
		(useNativeDialogs ? QFileDialog::Options() : QFileDialog::DontUseNativeDialog));
	// display file content
	QFile file(fileName);

	if(!file.isOpen()) {
		file.open(QIODevice::WriteOnly);
		QTextStream out(&file);
		out << m_codeEditor->toPlainText();

		m_console->appendPlainText("Save completed");
	} else {
		qDebug() << "File already opened! ";
		m_console->appendPlainText("File already opened! ");
	}

	if(file.isOpen())
		file.close();
}

void ScriptingTool::evaluateCode(QString code)
{
	QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
	if(!code.isEmpty()) {
		QString output;

		QJSValue val = ScopyJS::GetInstance()->engine()->evaluate(code, "");
		int ret = EXIT_SUCCESS;
		if(val.isError()) {
			qWarning(CAT_SCRIPTINGTOOL) << "Exception:" << val.toString();
			ret = EXIT_FAILURE;
			output += timestamp + ": Exception:" + val.toString();
		} else if(!val.isUndefined()) {
			qWarning(CAT_SCRIPTINGTOOL) << val.toString();
			output += timestamp + ": " + val.toString();
		}

		if(m_runBtn->isChecked())
			output += "\n" + timestamp + ": Script finished with status " + QString::number(ret);

		m_console->appendPlainText(output);
	} else {
		QString newValue = "\n" + timestamp + ": No input detected";
		m_console->appendPlainText(newValue);
	}

	if(m_runBtn->isEnabled()) {
		m_runBtn->setChecked(false);
	}
}

#include "moc_scriptingtool.cpp"
