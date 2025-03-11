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
#include <pluginbase/scopyjs.h>
#include <menuonoffswitch.h>
#include <style.h>

using namespace scopy;
using namespace scripting;

Q_LOGGING_CATEGORY(CAT_SCRIPTINGTOOL, "ScriptingTool")

ScriptingTool::ScriptingTool(QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *layout = new QVBoxLayout(this);
	this->setLayout(layout);

	QWidget *topWidget = new QWidget(this);

	QHBoxLayout *topWidgetLayout = new QHBoxLayout(topWidget);
	topWidget->setLayout(topWidgetLayout);

	QPushButton *loadBtn = new QPushButton("Load", topWidget);
	QPushButton *saveBtn = new QPushButton("Save", topWidget);
	MenuOnOffSwitch *toggleTerminalMode = new MenuOnOffSwitch("", topWidget);
	QPushButton *runBtn = new QPushButton("Run", topWidget);

	Style::setStyle(loadBtn, style::properties::button::basicButton);
	Style::setStyle(saveBtn, style::properties::button::basicButton);
	Style::setStyle(runBtn, style::properties::button::basicButton);

	topWidgetLayout->addWidget(loadBtn);
	topWidgetLayout->addWidget(saveBtn);
	topWidgetLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Maximum));
	topWidgetLayout->addWidget(toggleTerminalMode);
	topWidgetLayout->addWidget(runBtn);

	// TODO MAKE IT LOOK LIKE A CODE EDITOR
	QWidget *codeEditorWidget = new QWidget(this);
	QVBoxLayout *codeEditorLayout = new QVBoxLayout(codeEditorWidget);
	codeEditorLayout->setMargin(0);
	codeEditorLayout->setSpacing(0);

	Style::setStyle(codeEditorWidget, style::properties::widget::border_interactive);

	codeEditor = new ScopyCodeEditor(codeEditorWidget);
	codeEditorLayout->addWidget(codeEditor);

	QWidget *codeOutputWidget = new QWidget(this);
	QVBoxLayout *codeOutputLayout = new QVBoxLayout(codeOutputWidget);
	codeOutputLayout->setMargin(0);
	codeOutputLayout->setSpacing(0);

	Style::setStyle(codeOutputWidget, style::properties::widget::border_interactive);

	codeOutput = new QPlainTextEdit(this);
	codeOutput->setReadOnly(true);
	codeOutputLayout->addWidget(codeOutput);

	connect(loadBtn, &QPushButton::clicked, this, [=]() { loadFile(); });

	connect(saveBtn, &QPushButton::clicked, this, [=]() { saveToFile(); });

	connect(runBtn, &QPushButton::clicked, this, [=]() { compileCode(); });

	// TODO
	// connect(toggleTerminalMode, &QPushButton::toggled, this, [=](bool toggled){
	// });

	layout->addWidget(topWidget);
	layout->addWidget(codeEditorWidget, 4);
	layout->addWidget(codeOutputWidget), 2;
}

void ScriptingTool::loadFile()
{
	// chose file

	QString selectedFilter;
	fileName = QFileDialog::getSaveFileName(this, tr("Export"), "", tr("All Files(*)"), &selectedFilter,
						QFileDialog::Options(QFileDialog::DontUseNativeDialog));

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
		codeEditor->setPlainText(fileContent);
	} else {
		qDebug() << "File already opened! ";
		codeOutput->setPlainText("File already opened! ");
	}

	if(file.isOpen())
		file.close();
}

void ScriptingTool::saveToFile()
{
	QString selectedFilter;
	fileName = QFileDialog::getSaveFileName(this, tr("Export"), "", tr("All Files(*)"), &selectedFilter,
						QFileDialog::Options(QFileDialog::DontUseNativeDialog));

	// display file content
	QFile file(fileName);

	if(!file.isOpen()) {
		file.open(QIODevice::WriteOnly);
		QTextStream out(&file);
		out << codeEditor->toPlainText();

		codeOutput->setPlainText("Save completed");
	} else {
		qDebug() << "File already opened! ";
		codeOutput->setPlainText("File already opened! ");
	}

	if(file.isOpen())
		file.close();
}

void ScriptingTool::compileCode()
{
	if(!codeEditor->toPlainText().isEmpty()) {
		QString output;

		QJSValue val = ScopyJS::GetInstance()->engine()->evaluate(codeEditor->toPlainText(), "");
		int ret = EXIT_SUCCESS;
		if(val.isError()) {
			qWarning(CAT_SCRIPTINGTOOL) << "Exception:" << val.toString();
			ret = EXIT_FAILURE;
			output += "Exception:" + val.toString();
		} else if(!val.isUndefined()) {
			qWarning(CAT_SCRIPTINGTOOL) << val.toString();
			output += val.toString();
		}

		output += "\n Script finished with status " + QString::number(ret);

		QString newValue = "\n";
		newValue += output;
		newValue += "\n";
		newValue += codeOutput->toPlainText();
		codeOutput->setPlainText(newValue);
	} else {
		QString newValue = " \n No input detected";
		newValue += "\n";
		newValue += codeOutput->toPlainText();
		codeOutput->setPlainText(newValue);
	}
}
