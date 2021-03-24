/*
 * Copyright (c) 2021 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ui_scopy_color_editor.h"

#include <QColorDialog>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QPushButton>

#include <iostream>
#include <scopy/core/application_restarter.hpp>
#include <scopy/gui/scopy_color_editor.hpp>

void ScopyColorEditor::parseAndBuildMap(QString toParse)
{
	QStringList print = toParse.split("{").join("").split("}");

	QString textEditText = "";

	for (auto token : print) {
		// TODO: skip empty parts is only available in qt >= 5.14
		//		QStringList ttoken = token.split("\n", Qt::SkipEmptyParts);
		QStringList ttoken;
		if (ttoken.size()) {
			for (auto t : ttoken) {
				if (t.startsWith("/*"))
					continue; // ignore comments
				textEditText += t;
				textEditText += '\n';
				break;
			}

			int i = 0;
			QString mapKey = "";
			for (; i < ttoken.size(); ++i) {
				if (ttoken[i].startsWith("/*"))
					continue; // ignore comments
				mapKey = ttoken[i];
				break;
			}

			if (mapKey == "") {
				break;
			}

			m_entityStylesheetMap[mapKey] = QStringList();

			i++; // advance so we skip the key

			for (; i < ttoken.size(); ++i) {
				m_entityStylesheetMap[mapKey].push_back(ttoken[i]);
			}
		}
	}
}

ScopyColorEditor::ScopyColorEditor(QApplication* app, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::ScopyColorEditor)
	, m_app(app)
	, m_currentStylesheet("default")
	, m_sh(nullptr)
{
	m_ui->setupUi(this);

	m_ui->lineEditSearch->setAttribute(Qt::WA_MacShowFocusRect, false);

	QDir themes(":/stylesheets/themes/");
	QStringList stylesheets = themes.entryList();

	for (const QString& entry : stylesheets) {
		m_ui->stylesheetsCmbBox->addItem(entry);
	}

	connect(m_ui->btnCopy, &QPushButton::clicked, this, &ScopyColorEditor::copy);
	connect(m_ui->btnRemove, &QPushButton::clicked, this, &ScopyColorEditor::remove);
	connect(m_ui->stylesheetsCmbBox, &QComboBox::currentTextChanged, this, &ScopyColorEditor::stylesheetSelected);
	connect(m_ui->lineEditSearch, &QLineEdit::textChanged, this, &ScopyColorEditor::search);
	connect(m_ui->tabWidget, &QTabWidget::currentChanged, this, &ScopyColorEditor::tabChanged);
	connect(m_ui->btnNextMatch, &QPushButton::clicked, this, &ScopyColorEditor::nextMatch);
	connect(m_ui->btnPrevMatch, &QPushButton::clicked, this, &ScopyColorEditor::prevMatch);
	connect(m_ui->btnSave, &QPushButton::clicked, this, &ScopyColorEditor::advancedEditorChanged);
	connect(m_ui->textEditAdvancedEditor, &QTextEdit::textChanged, [=]() { m_ui->btnSave->setEnabled(true); });
	connect(m_ui->btnRestart, &QPushButton::clicked,
		[=]() { scopy::core::ApplicationRestarter::triggerRestart(); });

	m_sh = new SearchHighlight(m_ui->textEditAdvancedEditor->document());

	QFile file(":/stylesheets/themes/global.qss");
	file.open(QFile::ReadOnly);
	QString toParse = QString::fromLatin1(file.readAll());

	// disable color editor for now
	//	parseAndBuildMap(toParse);
	//	m_ui->textEditAdvancedEditor->setText(toParse);
	//	buildMenuForMap();
}

ScopyColorEditor::~ScopyColorEditor() { delete m_ui; }

QString ScopyColorEditor::getStyleSheet() const
{
	QString source = m_currentStylesheet;
	if (!m_userStylesheets.contains(source)) {
		source = ":/stylesheets/" + source + ".qss";
	}

	QFile file(source);
	file.open(QFile::ReadOnly);

	return QString::fromLatin1(file.readAll());
}

QString ScopyColorEditor::getCurrentStylesheet() const { return m_currentStylesheet; }

void ScopyColorEditor::setCurrentStylesheet(const QString& currentStylesheet)
{
	if (currentStylesheet.length()) {
		m_currentStylesheet = currentStylesheet;
	}

	//	m_app->setStyleSheet(getStyleSheet());

	//	m_ui->stylesheetsCmbBox->setCurrentText(m_currentStylesheet);

	//	const bool isUserStylesheet = m_userStylesheets.contains(m_currentStylesheet);

	//	m_ui->btnRemove->setEnabled(isUserStylesheet);
	//	m_ui->tabWidget->setVisible(isUserStylesheet);
	//	m_ui->lineEditSearch->setVisible(isUserStylesheet);
	//	m_ui->btnNextMatch->setVisible(isUserStylesheet);
	//	m_ui->btnPrevMatch->setVisible(isUserStylesheet);
}

QStringList ScopyColorEditor::getUserStylesheets() const { return m_userStylesheets; }

void ScopyColorEditor::setUserStylesheets(const QStringList& userStylesheets)
{
	m_userStylesheets = userStylesheets;
	//	for (const QString &entry : m_userStylesheets) {
	//		QFile entryFile(entry);
	//		if (!entryFile.exists()) { continue; }
	//		m_ui->stylesheetsCmbBox->addItem(entry);
	//	}
}

void ScopyColorEditor::buildMenuForMap()
{
	for (auto it = m_entityStylesheetMap.begin(); it != m_entityStylesheetMap.end(); ++it) {
		//		m_scrollArea->widget()->layout()->addWidget(new QPushButton(it.key()));
		auto layout = m_ui->scrollArea->widget()->layout();
		QWidget* colorEditor = new QWidget();
		colorEditor->setObjectName("colorEditor");
		QVBoxLayout* vBox = new QVBoxLayout();
		colorEditor->setLayout(vBox);
		vBox->addWidget(new QLabel(it.key()));
		colorEditor->setStyleSheet("QWidget#colorEditor{background-color: rgba(0, 0, 0, 60);}");

		//		std::cout << it.key().toStdString() << std::endl;

		bool toAdd = false;

		for (auto line : it.value()) {
			std::vector<QPushButton*> controls;
			int index = 0;
			while ((index = line.indexOf("rgba(", index)) != -1) {
				//				std::cout << "Found rgba( on line: " <<
				// line.toStdString()
				//<< " at index: " << index << std::endl;
				auto btn = new QPushButton(line.split(":")[0].replace(" ", ""));
				btn->setProperty("key", QVariant(it.key()));
				btn->setProperty("line", QVariant(line));
				btn->setProperty("index", QVariant(index));

				controls.push_back(btn);
				++index;
			}

			index = 0;
			while ((index = line.indexOf("#", index)) != -1) {
				//				std::cout << "Found # on line: " << line.toStdString()
				//<< " at index: " << index << std::endl;
				auto btn = new QPushButton(line.split(":")[0].replace(" ", ""));
				btn->setProperty("key", QVariant(it.key()));
				btn->setProperty("line", QVariant(line));
				btn->setProperty("index", QVariant(index));

				controls.push_back(btn);
				++index;
			}

			QHBoxLayout* btnLayout = nullptr;
			if (controls.size()) {
				btnLayout = new QHBoxLayout();
			}
			for (auto b : controls) {
				connect(b, &QPushButton::clicked, this, &ScopyColorEditor::changeColor);
				btnLayout->addWidget(b);
				toAdd = true;
			}
			if (btnLayout) {
				btnLayout->addSpacerItem(
					new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
				vBox->addLayout(btnLayout);
				vBox->addSpacerItem(
					new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding));
			}
		}

		if (toAdd) {
			m_colorEditors.append(colorEditor);
			layout->addWidget(colorEditor);
		}
	}
}

bool ScopyColorEditor::handleRgbaColor(const QString& key, const QString& line, int index, QPushButton* btn)
{
	int endIndex = -1;
	for (int i = index; i < line.size(); ++i) {
		if (line[i] == ")") {
			endIndex = i;
		}
	}

	std::cout << line[index].toLatin1() << " " << line[endIndex].toLatin1() << std::endl;

	QString colorString = "";
	for (int i = index; i <= endIndex; ++i) {
		colorString += line.at(i);
	}

	std::cout << colorString.toStdString() << std::endl;

	QStringList values = colorString.split("(")[1].split(")")[0].replace(" ", "").split(",");
	int r = 0, g = 0, b = 0, a = 0;

	r = values[0].toInt();
	g = values[1].toInt();
	b = values[2].toInt();
	if (values.size() == 4) {
		a = values[3].toInt();
	}

	std::cout << r << " " << g << " " << b << " " << a << std::endl;
	QColor oldColor = QColor();
	oldColor.setRed(r);
	oldColor.setGreen(g);
	oldColor.setBlue(b);
	oldColor.setAlpha(a);

	QColor newColor = QColorDialog::getColor(oldColor);
	if (!newColor.isValid()) {
		return false;
	}

	QString newColorString = "rgba(" + QString::number(newColor.red()) + ", " + QString::number(newColor.green()) +
		", " + QString::number(newColor.blue()) + ", " + QString::number(newColor.alpha()) + ")";

	std::cout << newColorString.toStdString() << std::endl;

	QStringList value = m_entityStylesheetMap[key];
	for (int i = 0; i < value.size(); ++i) {
		if (value[i] == line) {
			value[i].replace(index, endIndex - index + 1, newColorString);
			btn->setProperty("line", QVariant(value[i]));
		}
	}
	m_entityStylesheetMap[key] = value;

	return true;
}

bool ScopyColorEditor::handleHexColor(const QString& key, const QString& line, int index, QPushButton* btn)
{
	int endIndex = -1;
	for (int i = index; i < line.size(); ++i) {
		if (line[i] == " " || line[i] == ";") {
			endIndex = i;
		}
	}

	QString colorString = "";
	for (int i = index; i < endIndex; ++i) {
		colorString += line.at(i);
	}

	QColor oldColor(colorString);

	QColor newColor = QColorDialog::getColor(oldColor);
	if (!newColor.isValid()) {
		return false;
	}

	QString newColorString = newColor.name();

	QStringList value = m_entityStylesheetMap[key];
	for (int i = 0; i < value.size(); ++i) {
		if (value[i] == line) {
			value[i].replace(index, endIndex - index, newColorString);
			btn->setProperty("line", QVariant(value[i]));
		}
	}
	m_entityStylesheetMap[key] = value;

	return true;
}

void ScopyColorEditor::writeNewStylesheetToFile(QString stylesheet)
{
	QString source = m_currentStylesheet;
	if (!m_userStylesheets.contains(source)) {
		source = ":/stylesheets/themes/" + source;
	}
	QFile file(source);
	file.open(QFile::ReadWrite | QFile::Truncate | QFile::Text);
	QTextStream in(&file);
	in << stylesheet;
}

void ScopyColorEditor::rebuildAndApplyStylesheet()
{
	QString stylesheet = "";
	for (auto it = m_entityStylesheetMap.begin(); it != m_entityStylesheetMap.end(); ++it) {
		stylesheet += it.key() + " {\n";

		for (auto line : it.value()) {
			stylesheet += line + "\n";
		}

		stylesheet += "}\n\n";
	}

	m_app->setStyleSheet(stylesheet);

	m_ui->textEditAdvancedEditor->setText(stylesheet);

	writeNewStylesheetToFile(stylesheet);
}

void ScopyColorEditor::createNewFile() {}

void ScopyColorEditor::loadFile() {}

void ScopyColorEditor::changeColor()
{
	QPushButton* btn = qobject_cast<QPushButton*>(QObject::sender());

	QString key = btn->property("key").toString();
	QString line = btn->property("line").toString();
	int index = btn->property("index").toInt();

	bool changed = false;
	if (line[index] == "r") {
		changed = handleRgbaColor(key, line, index, btn);
	} else if (line[index] == "#") {
		changed = handleHexColor(key, line, index, btn);
	}

	if (changed) {
		rebuildAndApplyStylesheet();
	}
}

void ScopyColorEditor::copy()
{
	QStringList filter;
	filter += QString(tr("Qt stylesheet (*.qss)"));

	QString selectedFilter = filter[0];

	QString fileName = QFileDialog::getSaveFileName(this, tr("Copy"), "", filter.join(";;"), &selectedFilter,
							QFileDialog::Options());

	if (!fileName.length()) {
		return;
	}

	if (fileName.split(".").size() <= 1) {
		// file name w/o extension. Let's append it
		fileName += ".qss";
	}

	QString source = m_ui->stylesheetsCmbBox->currentText();
	if (!m_userStylesheets.contains(source)) {
		source = ":/stylesheets/themes/" + source;
	}
	qDebug() << QFile::copy(source, fileName);

	m_userStylesheets.append(fileName);
	m_ui->stylesheetsCmbBox->addItem(fileName);
}

void ScopyColorEditor::remove()
{
	QString file = m_ui->stylesheetsCmbBox->currentText();
	if (m_userStylesheets.contains(file)) {
		m_userStylesheets.removeOne(file);
		m_ui->stylesheetsCmbBox->removeItem(m_ui->stylesheetsCmbBox->currentIndex());
	}
}

QString ScopyColorEditor::clearAndRebuildEditor(const QString& stylesheet)
{
	m_entityStylesheetMap.clear();
	auto layout = m_ui->scrollArea->widget()->layout();
	for (QWidget* widget : m_colorEditors) {
		layout->removeWidget(widget);
		widget->deleteLater();
	}

	m_colorEditors.clear();

	QString source = stylesheet;
	if (!m_userStylesheets.contains(source)) {
		source = ":/stylesheets/themes/" + source;
	}
	QFile file(source);
	file.open(QFile::ReadOnly);
	QString toParse = QString::fromLatin1(file.readAll());
	parseAndBuildMap(toParse);
	buildMenuForMap();

	return source;
}

void ScopyColorEditor::stylesheetSelected(const QString& stylesheet)
{
	const bool isUserStylesheet = m_userStylesheets.contains(stylesheet);

	m_ui->btnRemove->setEnabled(isUserStylesheet);
	m_ui->tabWidget->setVisible(isUserStylesheet);
	m_ui->lineEditSearch->setVisible(isUserStylesheet);
	m_ui->btnNextMatch->setVisible(isUserStylesheet);
	m_ui->btnPrevMatch->setVisible(isUserStylesheet);

	QString source = clearAndRebuildEditor(stylesheet);
	QFile file(source);
	file.open(QFile::ReadOnly);
	QString toParse = QString::fromLatin1(file.readAll());
	m_ui->textEditAdvancedEditor->setText(toParse);

	rebuildAndApplyStylesheet();

	m_currentStylesheet = stylesheet;
}

void ScopyColorEditor::search(const QString& searchText)
{
	for (QWidget* w : m_colorEditors) {
		QLabel* label = qobject_cast<QLabel*>(w->layout()->itemAt(0)->widget());
		bool hasSearchedWords = label->text().contains(searchText);
		w->setVisible(hasSearchedWords);
	}

	m_sh->searchText(searchText);
}

void ScopyColorEditor::tabChanged(int index)
{
	if (m_ui->tabWidget->widget(index) != m_ui->widgetAdvancedEditor) {
		m_ui->btnNextMatch->setDisabled(true);
		m_ui->btnPrevMatch->setDisabled(true);
	} else {
		m_ui->btnNextMatch->setEnabled(true);
		m_ui->btnPrevMatch->setEnabled(true);
	}
}

void ScopyColorEditor::nextMatch()
{
	m_ui->textEditAdvancedEditor->find(m_ui->lineEditSearch->text(), QTextDocument::FindCaseSensitively);
	m_ui->textEditAdvancedEditor->ensureCursorVisible();
	m_sh->rehighlight();
}

void ScopyColorEditor::prevMatch()
{
	m_ui->textEditAdvancedEditor->find(m_ui->lineEditSearch->text(),
					   QTextDocument::FindCaseSensitively | QTextDocument::FindBackward);
	m_ui->textEditAdvancedEditor->ensureCursorVisible();
	m_sh->rehighlight();
}

void ScopyColorEditor::advancedEditorChanged()
{
	writeNewStylesheetToFile(m_ui->textEditAdvancedEditor->toPlainText());
	clearAndRebuildEditor(m_currentStylesheet);
	rebuildAndApplyStylesheet();
	m_ui->btnSave->setDisabled(true);
}

SearchHighlight::SearchHighlight(QTextDocument* parent)
	: QSyntaxHighlighter(parent)
{
	m_format.setBackground(Qt::green);
}

void SearchHighlight::searchText(const QString& text)
{
	m_pattern = QRegularExpression(text);
	rehighlight();
	;
}

void SearchHighlight::highlightBlock(const QString& text)
{
	QRegularExpressionMatchIterator matchIterator = m_pattern.globalMatch(text);
	while (matchIterator.hasNext()) {
		QRegularExpressionMatch match = matchIterator.next();
		//		qDebug() << "Match at: " << match.capturedStart();
		setFormat(match.capturedStart(), match.capturedLength(), m_format);
	}
}
