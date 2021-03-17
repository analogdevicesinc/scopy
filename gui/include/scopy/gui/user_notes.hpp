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

#ifndef USER_NOTES_HPP
#define USER_NOTES_HPP

#include <QButtonGroup>
#include <QPushButton>
#include <QSettings>
#include <QTextBrowser>
#include <QWidget>

#include <scopy/core/api_object.hpp>
#include <vector>

namespace Ui {
class UserNotes;
class Note;
class UserNotePage;
} // namespace Ui

namespace scopy {
namespace gui {

class UserNotes_API;
class Note;
class Note_API;

class UserNotes : public QWidget
{
	friend class UserNotes_API;

	Q_OBJECT

public:
	explicit UserNotes(QWidget* parent = 0);
	~UserNotes();

	scopy::core::ApiObject* getApi();

public Q_SLOTS:
	void addBtnClicked(bool);
	void browseBtnClicked(bool);
	void saveBtnClicked(bool);
	void removeBtnClicked(bool);

private Q_SLOTS:
	void noteSelected(bool);
	void pageMoved(int);

private:
	void loadPageForNote(Note* note, QString path);
	Note* getSelectedNote();
	int getNoteIndex(Note* note);

	Note* addNote(QString name, QString path);
	void clearAllNotes();

private:
	Ui::UserNotes* m_ui;
	UserNotes_API* m_notesApi;
	std::vector<Note*> m_notes;
	QButtonGroup* m_notesGroup;
	int m_noteCount;
};

class Note : public QWidget
{
	Q_OBJECT

public:
	explicit Note(QString name, QString path, QWidget* parent = 0);
	~Note();

	QString getName() const;
	void setName(const QString& name);

	QString getPath() const;
	void setPath(const QString& path);

	bool getSelected() const;
	QPushButton* noteButton();

	void click();

	QWidget* getPage() const;
	void setPage(QWidget* page);

	Ui::UserNotePage* getPageUi();

public Q_SLOTS:
	void setSelected(bool selected);

Q_SIGNALS:
	void selected(bool);

private:
	Ui::Note* m_ui;
	QString m_name;
	QString m_path;
	bool m_selected;
	QWidget* m_page;
	Ui::UserNotePage* m_pageUi;
};
} // namespace gui
} // namespace scopy

#endif // USER_NOTES_HPP
