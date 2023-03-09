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
class UserNotePage;
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

        QWidget *getPage() const;
        void setPage(QWidget *page);

        Ui::UserNotePage *getPageUi();

public Q_SLOTS:
        void setSelected(bool selected);

Q_SIGNALS:
        void selected(bool);

private:
        Ui::Note *ui;
        QString m_name;
        QString m_path;
        bool m_selected;
        QWidget *m_page;
        Ui::UserNotePage *pageUi;
};
}
#endif // USER_NOTES_HPP
