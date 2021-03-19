/*
 * Copyright (c) 2019 Analog Devices Inc.
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

#include "dynamic_widget.hpp"
#include "user_notes_api.hpp"

#include "ui_note.h"
#include "ui_user_note_page.h"
#include "ui_user_notes.h"

#include <QFileDialog>
#include <QFileInfo>

#include <scopy/gui/user_notes.hpp>

using namespace scopy::gui;

UserNotes::UserNotes(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::UserNotes)
	, m_notesApi(new UserNotes_API(this))
	, m_notesGroup(new QButtonGroup(this))
	, m_noteCount(0)
{
	m_ui->setupUi(this);

	connect(m_ui->btnAdd, SIGNAL(toggled(bool)), this, SLOT(addBtnClicked(bool)));
	connect(m_ui->btnBrowse, SIGNAL(clicked(bool)), SLOT(browseBtnClicked(bool)));
	connect(m_ui->btnSave, SIGNAL(clicked(bool)), SLOT(saveBtnClicked(bool)));
	connect(m_ui->stackedWidget, SIGNAL(moved(int)), this, SLOT(pageMoved(int)));

	m_notesGroup->addButton(m_ui->btnAdd);
	m_ui->btnAdd->setChecked(true);
	m_ui->lineEditPath->setReadOnly(true);

	m_notesApi->setObjectName(QString("notes"));

	QSettings oldSettings;
	QFile scopy(oldSettings.fileName());
	QFile tempFile(oldSettings.fileName() + ".bak");
	QSettings settings(tempFile.fileName(), QSettings::IniFormat);

	m_notesApi->load(settings);

	m_ui->stackedWidget->setControlsEnabled(false);
}

UserNotes::~UserNotes()
{
	QSettings oldSettings;
	QFile scopy(oldSettings.fileName());
	QFile tempFile(oldSettings.fileName() + ".bak");
	QSettings settings(tempFile.fileName(), QSettings::IniFormat);

	m_notesApi->save(settings);
	delete m_notesApi;

	for (auto n : m_notes) {
		delete n;
	}
	m_notes.clear();

	delete m_ui;
}

scopy::core::ApiObject* UserNotes::getApi() { return m_notesApi; }

void UserNotes::addBtnClicked(bool clicked)
{
	DynamicWidget::setDynamicProperty(m_ui->widgetAdd, "selected", clicked);
	if (clicked) {
		m_ui->stackedWidget->slideToIndex(0);
	}
}

void UserNotes::browseBtnClicked(bool clicked)
{
	/* Cleanup warnings related to old path */
	DynamicWidget::setDynamicProperty(m_ui->lineEditPath, "invalid", false);
	m_ui->lblPathWarning->setText("");

	QString fileName = QFileDialog::getOpenFileName(this, tr("Import"), "", tr("Html (*.html);;"), nullptr);

	if (fileName.length()) {
		m_ui->lineEditPath->setText(fileName);
		m_ui->lineEditPath->setStyleSheet("");
	}
}

void UserNotes::saveBtnClicked(bool clicked)
{
	if (m_ui->lineEditName->text() == "") {
		m_ui->lineEditName->setText(tr("Note ") + QString::number(m_noteCount));
	}

	if (m_ui->lineEditPath->text() == "") {
		m_ui->lineEditPath->setStyleSheet("border: 1px solid red");
		return;
	}

	auto note = addNote(m_ui->lineEditName->text(), m_ui->lineEditPath->text());

	if (note->getPage()) {
		DynamicWidget::setDynamicProperty(m_ui->lineEditPath, "invalid", false);
		m_ui->lblPathWarning->setText("");
		m_ui->lineEditPath->clear();
		m_ui->lineEditName->clear();
		note->click();
	} else {
		DynamicWidget::setDynamicProperty(m_ui->lineEditPath, "invalid", true);
		m_ui->lblPathWarning->setText(tr(" Invalid or empty file!"));
	}
}

Note* UserNotes::getSelectedNote()
{
	for (auto n : m_notes) {
		if (n->getSelected()) {
			return n;
		}
	}
	return nullptr;
}

int UserNotes::getNoteIndex(Note* note)
{
	if (!note)
		return -1;
	for (int i = 0; i < m_notes.size(); i++) {
		if (m_notes.at(i) == note) {
			return i;
		}
	}
	return -1;
}

void UserNotes::noteSelected(bool selected)
{
	if (selected) {
		Note* n = getSelectedNote();
		if (n) {
			loadPageForNote(n, n->getPath());
			m_ui->stackedWidget->slideToIndex(getNoteIndex(getSelectedNote()) + 1);
		} else {
			m_ui->stackedWidget->slideToIndex(0);
		}
	}
}

void UserNotes::removeBtnClicked(bool toggled)
{
	if (getSelectedNote()) {
		auto n = getSelectedNote();
		int pos = getNoteIndex(getSelectedNote());
		m_ui->stackedWidget->removeWidget(n->getPage());
		m_notesGroup->removeButton(n->noteButton());
		if (pos > 0) {
			m_notes.at(pos - 1)->click();
		} else {
			m_ui->btnAdd->click();
		}
		delete n;
		m_notes.erase(m_notes.begin() + pos);
	}
}

void UserNotes::pageMoved(int direction)
{
	if (m_ui->btnAdd->isChecked()) {
		(direction > 0) ? m_notes.at(0)->click() : m_ui->btnAdd->click();
	} else {
		int selectedIdx = getNoteIndex(getSelectedNote()) + direction;
		if (selectedIdx < (int)m_notes.size()) {
			(selectedIdx >= 0) ? m_notes.at(selectedIdx)->click() : m_ui->btnAdd->click();
		}
	}
}

Note* UserNotes::addNote(QString name, QString path)
{
	Note* newNote = new Note(name, path, this);

	connect(newNote->getPageUi()->btnRemove, SIGNAL(clicked(bool)), this, SLOT(removeBtnClicked(bool)));

	connect(newNote, SIGNAL(selected(bool)), this, SLOT(noteSelected(bool)));

	m_notesGroup->addButton(newNote->noteButton());
	m_ui->notesList->insertWidget(m_ui->notesList->count() - 1, newNote);
	m_notes.push_back(newNote);
	m_noteCount++;
	loadPageForNote(newNote, path);
	return newNote;
}

void UserNotes::loadPageForNote(Note* note, QString path)
{
	int pos = getNoteIndex(note);
	pos++;

	QTextBrowser* index = note->getPageUi()->textBrowser;
	if (!index) {
		index = new QTextBrowser(m_ui->stackedWidget);
	}
	index->setFrameShape(QFrame::NoFrame);

	QFileInfo fileInfo(path);
	if (fileInfo.exists()) {
		QFile indexFile(path);
		indexFile.open(QFile::ReadOnly);
		if (!indexFile.readAll().isEmpty()) {
			index->clear();
			index->setSearchPaths(QStringList(fileInfo.dir().absolutePath()));
			indexFile.close();
			index->setSource(QUrl::fromLocalFile(fileInfo.filePath()));
			index->setOpenExternalLinks(true);
			note->getPageUi()->lbl->setText(tr("Path: ") + path);
		} else {
			indexFile.close();
			index->clear();
			note->getPageUi()->lbl->setText(tr("Warning: The file is empty!"));
		}
	} else {
		index->clear();
		note->getPageUi()->lbl->setText(tr("Warning: The path is invalid!"));
	}

	if (index) {
		m_ui->stackedWidget->insertWidget(pos, note->getPage());
	}
}

void UserNotes::clearAllNotes()
{
	m_ui->btnAdd->click();
	for (auto n : m_notes) {
		delete n;
	}
	m_notes.clear();
}

/*
 * Note class
 */

Note::Note(QString name, QString path, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::Note)
	, m_selected(false)
	, m_name(name)
	, m_path(path)
	, m_page(nullptr)
{
	m_ui->setupUi(this);
	m_ui->lblName->setText(name);
	connect(m_ui->btn, SIGNAL(toggled(bool)), this, SLOT(setSelected(bool)));

	m_pageUi = new Ui::UserNotePage();
	m_page = new QWidget(this);
	m_pageUi->setupUi(m_page);
}

Note::~Note()
{
	if (m_page) {
		delete m_page;
		m_page = nullptr;
	}

	delete m_pageUi;
	delete m_ui;
}

Ui::UserNotePage* Note::getPageUi() { return m_pageUi; }

QString Note::getName() const { return m_name; }

void Note::setName(const QString& name)
{
	m_name = name;
	m_ui->lblName->setText(name);
}

QString Note::getPath() const { return m_path; }

void Note::setPath(const QString& path) { m_path = path; }

bool Note::getSelected() const { return m_selected; }

void Note::setSelected(bool sel)
{
	m_selected = sel;
	DynamicWidget::setDynamicProperty(m_ui->widget, "selected", sel);
	Q_EMIT selected(sel);
}

QWidget* Note::getPage() const { return m_page; }

void Note::setPage(QWidget* page) { m_page = page; }

QPushButton* Note::noteButton() { return m_ui->btn; }

void Note::click() { m_ui->btn->setChecked(true); }
