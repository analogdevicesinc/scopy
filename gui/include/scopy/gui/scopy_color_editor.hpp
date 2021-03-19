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

#ifndef SCOPY_COLOR_EDITOR_H
#define SCOPY_COLOR_EDITOR_H

#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextDocument>
#include <QTextEdit>
#include <QWidget>

namespace Ui {
class ScopyColorEditor;
}

class SearchHighlight : public QSyntaxHighlighter
{
	Q_OBJECT
public:
	explicit SearchHighlight(QTextDocument* parent = nullptr);

	void searchText(const QString& text);

protected:
	virtual void highlightBlock(const QString& text) override;

private:
	QRegularExpression m_pattern;
	QTextCharFormat m_format;
};

class ScopyColorEditor : public QWidget
{
	Q_OBJECT

public:
	explicit ScopyColorEditor(QApplication* app, QWidget* parent = nullptr);
	~ScopyColorEditor();

	QString getStyleSheet() const;

	QString getCurrentStylesheet() const;
	void setCurrentStylesheet(const QString& currentStylesheet);

	QStringList getUserStylesheets() const;
	void setUserStylesheets(const QStringList& userStylesheets);

private:
	void buildMenuForMap();
	bool handleRgbaColor(const QString& key, const QString& line, int index, QPushButton* btn);
	bool handleHexColor(const QString& key, const QString& line, int index, QPushButton* btn);

	void rebuildAndApplyStylesheet();

	void createNewFile();
	void loadFile();
	void parseAndBuildMap(QString toParse);
	void writeNewStylesheetToFile(QString stylesheet);

private Q_SLOTS:
	void changeColor();
	void copy();
	void remove();
	void stylesheetSelected(const QString& stylesheet);
	void search(const QString& searchText);
	void tabChanged(int index);
	void nextMatch();
	void prevMatch();
	void advancedEditorChanged();

private:
	Ui::ScopyColorEditor* m_ui;

	QVector<QWidget*> m_colorEditors;
	QMap<QString, QStringList> m_entityStylesheetMap;
	QScrollArea* m_scrollArea{nullptr};
	QTextEdit* m_textEdit{nullptr};
	QApplication* m_app;

	QString m_currentStylesheet;
	QStringList m_userStylesheets;

	SearchHighlight* m_sh;
	QString clearAndRebuildEditor(const QString& stylesheet);
};

#endif // SCOPY_COLOR_EDITOR_H
