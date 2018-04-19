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

#ifndef USER_NOTES_HPP
#define USER_NOTES_HPP

#include <QWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QTextBrowser>
#include <QSettings>
#include <vector>

#include "apiObject.hpp"

using namespace std;

namespace Ui {
class UserNotes;
class Note;
}

namespace adiscope  {
class UserNotes_API;
class Note;
class Note_API;

class UserNotes : public QWidget
{
        friend class UserNotes_API;

        Q_OBJECT

public:
        explicit UserNotes(QWidget *parent = 0);
        ~UserNotes();

	ApiObject *api();

public Q_SLOTS:
	void add_btn_clicked(bool);
	void browse_btn_clicked(bool);
	void save_btn_clicked(bool);
	void remove_btn_clicked(bool);

private Q_SLOTS:
        void note_selected(bool);
        void pageMoved(int);

private:
        void loadPageForNote(Note *note, QString path);
        Note* getSelectedNote();
        int getNoteIndex(Note *note);

        Note* addNote(QString name, QString path);
        void clearAllNotes();

private:
        Ui::UserNotes *ui;
        UserNotes_API *notes_api;
        vector<Note*> m_notes;
        QButtonGroup *notes_group;
        int m_note_count;
};


class Note : public QWidget
{
        Q_OBJECT

public:
        explicit Note(QString name, QString path, QWidget *parent = 0);
        ~Note();

        QString getName() const;
        void setName(const QString &name);

        QString getPath() const;
        void setPath(const QString &path);

        bool getSelected() const;
        QPushButton* noteButton();

        void click();

        QTextBrowser *getPage() const;
        void setPage(QTextBrowser *page);

public Q_SLOTS:
        void setSelected(bool selected);

Q_SIGNALS:
        void selected(bool);

private:
        Ui::Note *ui;
        QString m_name;
        QString m_path;
        bool m_selected;
        QTextBrowser *m_page;
};


class UserNotes_API : public ApiObject
{
        Q_OBJECT

	Q_PROPERTY(int notes_list_size READ notes_list_size WRITE
		   set_notes_list_size SCRIPTABLE false)
	Q_PROPERTY(QVariantList notes READ getNotes)

public:
        explicit UserNotes_API(UserNotes *notesPanel) :
                ApiObject(),
                notesPanel(notesPanel) {}
        ~UserNotes_API() {
                qDeleteAll(notes_list_api);
                notes_list_api.clear();
        }

	int notes_list_size();
	void set_notes_list_size(int size);

	QVariantList getNotes();
private:
	UserNotes *notesPanel;
	QList<Note_API*> notes_list_api;
	void refreshApi();
};

class Note_API : public ApiObject
{
        Q_OBJECT

        Q_PROPERTY(QString name READ getName WRITE setName);
        Q_PROPERTY(QString path READ getPath WRITE setPath);

public:
        explicit Note_API(Note *note) :
                ApiObject(),
                note(note) {}
        ~Note_API() {}

	QString getName();
	void setName(QString);

	QString getPath();
	void setPath(QString);

private:
	Note *note;

};
}
#endif // USER_NOTES_HPP
