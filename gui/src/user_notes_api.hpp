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
#ifndef USER_NOTES_API_HPP
#define USER_NOTES_API_HPP

#include <scopy/gui/user_notes.hpp>

namespace scopy {
namespace gui {

class UserNotes_API : public scopy::core::ApiObject
{
	Q_OBJECT

	Q_PROPERTY(int notesListSize READ notesListSize WRITE setNotesListSize SCRIPTABLE false)
	Q_PROPERTY(QVariantList notes READ getNotes)

public:
	explicit UserNotes_API(UserNotes* notesPanel)
		: ApiObject()
		, m_notesPanel(notesPanel)
	{}
	~UserNotes_API()
	{
		qDeleteAll(m_notesListApi);
		m_notesListApi.clear();
	}

	int notesListSize();
	void setNotesListSize(int size);

	QVariantList getNotes();

private:
	UserNotes* m_notesPanel;
	QList<Note_API*> m_notesListApi;

	void refreshApi();
};

class Note_API : public scopy::core::ApiObject
{
	Q_OBJECT

	Q_PROPERTY(QString name READ getName WRITE setName);
	Q_PROPERTY(QString path READ getPath WRITE setPath);

public:
	explicit Note_API(Note* note)
		: ApiObject()
		, m_note(note)
	{}
	~Note_API() {}

	QString getName();
	void setName(QString);

	QString getPath();
	void setPath(QString);

private:
	Note* m_note;
};
} // namespace gui
} // namespace scopy

#endif // USER_NOTES_API_HPP
