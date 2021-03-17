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
#include "user_notes_api.hpp"

using namespace scopy::gui;

/*
 * Note_API class
 */
void Note_API::setPath(QString path)
{
	m_note->setPath(path);
}

QString Note_API::getPath()
{
	return m_note->getPath();
}

void Note_API::setName(QString name)
{
	m_note->setName(name);
}

QString Note_API::getName()
{
	return m_note->getName();
}


/*
 * UserNotes_API class
 */

int UserNotes_API::notesListSize()
{
	refreshApi();
	return m_notesListApi.size();
}

void UserNotes_API::setNotesListSize(int size)
{
	m_notesPanel->clearAllNotes();
	for (int i = 0; i < size; i++){
		auto n = m_notesPanel->addNote("", "");
	}
}

QVariantList UserNotes_API::getNotes()
{
	QVariantList list;
	refreshApi();

	for (auto *each : m_notesListApi) {
		list.append(QVariant::fromValue(each));
	}
	return list;
}

void UserNotes_API::refreshApi()
{
	qDeleteAll(m_notesListApi);
	m_notesListApi.clear();
	for (auto note : m_notesPanel->m_notes) {
		m_notesListApi.append(new Note_API(note));
	}
}
