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

namespace adiscope {

/*
 * Note_API class
 */
void Note_API::setPath(QString path)
{
	note->setPath(path);
}

QString Note_API::getPath()
{
	return note->getPath();
}

void Note_API::setName(QString name)
{
	note->setName(name);
}

QString Note_API::getName()
{
	return note->getName();
}


/*
 * UserNotes_API class
 */

int UserNotes_API::notes_list_size()
{
	refreshApi();
	return notes_list_api.size();
}

void UserNotes_API::set_notes_list_size(int size)
{
	notesPanel->clearAllNotes();
	for (int i = 0; i < size; i++){
		notesPanel->addNote("", "");
	}
}

QVariantList UserNotes_API::getNotes()
{
	QVariantList list;
	refreshApi();

	for (auto *each : qAsConst(notes_list_api)) {
		list.append(QVariant::fromValue(each));
	}
	return list;
}

void UserNotes_API::refreshApi()
{
	qDeleteAll(notes_list_api);
	notes_list_api.clear();
	for (auto note : notesPanel->m_notes) {
		notes_list_api.append(new Note_API(note));
	}
}

}
