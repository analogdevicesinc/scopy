/*
 * Copyright 2018 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "user_notes.hpp"

#include "dynamicWidget.hpp"
#include "user_notes_api.hpp"

#include "ui_note.h"
#include "ui_user_note_page.h"
#include "ui_user_notes.h"

#include <QFileDialog>
#include <QFileInfo>

using namespace adiscope;

UserNotes::UserNotes(QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::UserNotes)
	, notes_api(new UserNotes_API(this))
	, notes_group(new QButtonGroup(this))
	, m_note_count(0) {
	ui->setupUi(this);

	connect(ui->addBtn, SIGNAL(toggled(bool)), this, SLOT(add_btn_clicked(bool)));
	connect(ui->browseBtn, SIGNAL(clicked(bool)), SLOT(browse_btn_clicked(bool)));
	connect(ui->saveBtn, SIGNAL(clicked(bool)), SLOT(save_btn_clicked(bool)));
	connect(ui->stackedWidget, SIGNAL(moved(int)), this, SLOT(pageMoved(int)));

	notes_group->addButton(ui->addBtn);
	ui->addBtn->setChecked(true);
	ui->pathLineEdit->setReadOnly(true);

	notes_api->setObjectName(QString("notes"));

	QSettings oldSettings;
	QFile scopy(oldSettings.fileName());
	QFile tempFile(oldSettings.fileName() + ".bak");
	QSettings settings(tempFile.fileName(), QSettings::IniFormat);

	notes_api->load(settings);
	ui->stackedWidget->setStyleSheet("background-color: black;");

	ui->stackedWidget->set_controls_enabled(false);
}

UserNotes::~UserNotes() {
	QSettings oldSettings;
	QFile scopy(oldSettings.fileName());
	QFile tempFile(oldSettings.fileName() + ".bak");
	QSettings settings(tempFile.fileName(), QSettings::IniFormat);

	notes_api->save(settings);
	delete notes_api;

	for (auto n : m_notes) {
		delete n;
	}
	m_notes.clear();

	delete ui;
}

ApiObject *UserNotes::api() { return notes_api; }

void UserNotes::add_btn_clicked(bool clicked) {
	setDynamicProperty(ui->addWidget, "selected", clicked);
	if (clicked) {
		ui->stackedWidget->slideToIndex(0);
	}
}

void UserNotes::browse_btn_clicked(bool clicked) {
	/* Cleanup warnings related to old path */
	setDynamicProperty(ui->pathLineEdit, "invalid", false);
	ui->pathWarning->setText("");

	QString pathToFile = "";
	auto export_dialog(new QFileDialog(this));
	export_dialog->setWindowModality(Qt::WindowModal);
	export_dialog->setFileMode(QFileDialog::AnyFile);
	export_dialog->setAcceptMode(QFileDialog::AcceptOpen);
	export_dialog->setNameFilters({"(*.html)"});
	if (export_dialog->exec()) {
		QFile f(export_dialog->selectedFiles().at(0));
		pathToFile = f.fileName();
		ui->pathLineEdit->setText(pathToFile);
		ui->pathLineEdit->setStyleSheet("");
	}
}

void UserNotes::save_btn_clicked(bool clicked) {
	if (ui->nameLineEdit->text() == "") {
		ui->nameLineEdit->setText("Note " + QString::number(m_note_count));
	}

	if (ui->pathLineEdit->text() == "") {
		ui->pathLineEdit->setStyleSheet("border: 1px solid red");
		return;
	}

	auto note = addNote(ui->nameLineEdit->text(), ui->pathLineEdit->text());

	if (note->getPage()) {
		setDynamicProperty(ui->pathLineEdit, "invalid", false);
		ui->pathWarning->setText("");
		ui->pathLineEdit->clear();
		ui->nameLineEdit->clear();
		note->click();
	} else {
		setDynamicProperty(ui->pathLineEdit, "invalid", true);
		ui->pathWarning->setText(" Invalid or empty file!");
	}
}

Note *UserNotes::getSelectedNote() {
	for (auto n : m_notes) {
		if (n->getSelected()) {
			return n;
		}
	}
	return nullptr;
}

int UserNotes::getNoteIndex(Note *note) {
	if (!note)
		return -1;
	for (int i = 0; i < m_notes.size(); i++) {
		if (m_notes.at(i) == note) {
			return i;
		}
	}
	return -1;
}

void UserNotes::note_selected(bool selected) {
	if (selected) {
		Note *n = getSelectedNote();
		if (n) {
			loadPageForNote(n, n->getPath());
			ui->stackedWidget->slideToIndex(getNoteIndex(getSelectedNote()) + 1);
		} else {
			ui->stackedWidget->slideToIndex(0);
		}
	}
}

void UserNotes::remove_btn_clicked(bool toggled) {
	if (getSelectedNote()) {
		auto n = getSelectedNote();
		int pos = getNoteIndex(getSelectedNote());
		ui->stackedWidget->removeWidget(n->getPage());
		notes_group->removeButton(n->noteButton());
		if (pos > 0) {
			m_notes.at(pos - 1)->click();
		} else {
			ui->addBtn->click();
		}
		delete n;
		m_notes.erase(m_notes.begin() + pos);
	}
}

void UserNotes::pageMoved(int direction) {
	if (ui->addBtn->isChecked()) {
		(direction > 0) ? m_notes.at(0)->click() : ui->addBtn->click();
	} else {
		int selectedIdx = getNoteIndex(getSelectedNote()) + direction;
		if (selectedIdx < (int)m_notes.size()) {
			(selectedIdx >= 0) ? m_notes.at(selectedIdx)->click() : ui->addBtn->click();
		}
	}
}

Note *UserNotes::addNote(QString name, QString path) {
	Note *newNote = new Note(name, path, this);

	connect(newNote->getPageUi()->btnRemove, SIGNAL(clicked(bool)), this, SLOT(remove_btn_clicked(bool)));

	connect(newNote, SIGNAL(selected(bool)), this, SLOT(note_selected(bool)));

	notes_group->addButton(newNote->noteButton());
	ui->notesList->insertWidget(ui->notesList->count() - 1, newNote);
	m_notes.push_back(newNote);
	m_note_count++;
	loadPageForNote(newNote, path);
	return newNote;
}

void UserNotes::loadPageForNote(Note *note, QString path) {
	int pos = getNoteIndex(note);
	pos++;

	QTextBrowser *index = note->getPageUi()->textBrowser;
	if (!index) {
		index = new QTextBrowser(ui->stackedWidget);
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
			note->getPageUi()->label->setText("Path: " + path);
		} else {
			indexFile.close();
			index->clear();
			note->getPageUi()->label->setText("Warning: The file is empty!");
		}
	} else {
		index->clear();
		note->getPageUi()->label->setText("Warning: The path is invalid!");
	}

	if (index) {
		ui->stackedWidget->insertWidget(pos, note->getPage());
	}
}

void UserNotes::clearAllNotes() {
	ui->addBtn->click();
	for (auto n : m_notes) {
		delete n;
	}
	m_notes.clear();
}

/*
 * Note class
 */

Note::Note(QString name, QString path, QWidget *parent)
	: QWidget(parent), ui(new Ui::Note), m_selected(false), m_name(name), m_path(path), m_page(nullptr) {
	ui->setupUi(this);
	ui->name->setText(name);
	connect(ui->btn, SIGNAL(toggled(bool)), this, SLOT(setSelected(bool)));

	pageUi = new Ui::UserNotePage();
	m_page = new QWidget(this);
	pageUi->setupUi(m_page);
}

Note::~Note() {
	if (m_page) {
		delete m_page;
		m_page = nullptr;
	}

	delete pageUi;
	delete ui;
}

Ui::UserNotePage *Note::getPageUi() { return pageUi; }

QString Note::getName() const { return m_name; }

void Note::setName(const QString &name) {
	m_name = name;
	ui->name->setText(name);
}

QString Note::getPath() const { return m_path; }

void Note::setPath(const QString &path) { m_path = path; }

bool Note::getSelected() const { return m_selected; }

void Note::setSelected(bool sel) {
	m_selected = sel;
	setDynamicProperty(ui->widget, "selected", sel);
	Q_EMIT selected(sel);
}

QWidget *Note::getPage() const { return m_page; }

void Note::setPage(QWidget *page) { m_page = page; }

QPushButton *Note::noteButton() { return ui->btn; }

void Note::click() { ui->btn->setChecked(true); }
