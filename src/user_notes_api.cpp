#include "user_notes_api.hpp"

namespace adiscope {

/*
 * Note_API class
 */
void Note_API::setPath(QString path) { note->setPath(path); }

QString Note_API::getPath() { return note->getPath(); }

void Note_API::setName(QString name) { note->setName(name); }

QString Note_API::getName() { return note->getName(); }

/*
 * UserNotes_API class
 */

int UserNotes_API::notes_list_size() {
	refreshApi();
	return notes_list_api.size();
}

void UserNotes_API::set_notes_list_size(int size) {
	notesPanel->clearAllNotes();
	for (int i = 0; i < size; i++) {
		auto n = notesPanel->addNote("", "");
	}
}

QVariantList UserNotes_API::getNotes() {
	QVariantList list;
	refreshApi();

	for (auto *each : notes_list_api) {
		list.append(QVariant::fromValue(each));
	}
	return list;
}

void UserNotes_API::refreshApi() {
	qDeleteAll(notes_list_api);
	notes_list_api.clear();
	for (auto note : notesPanel->m_notes) {
		notes_list_api.append(new Note_API(note));
	}
}

} // namespace adiscope
