#ifndef USER_NOTES_API_HPP
#define USER_NOTES_API_HPP

#include "user_notes.hpp"

namespace adiscope {

class UserNotes_API : public ApiObject {
	Q_OBJECT

	Q_PROPERTY(int notes_list_size READ notes_list_size WRITE set_notes_list_size SCRIPTABLE false)
	Q_PROPERTY(QVariantList notes READ getNotes)

public:
	explicit UserNotes_API(UserNotes *notesPanel) : ApiObject(), notesPanel(notesPanel) {}
	~UserNotes_API() {
		qDeleteAll(notes_list_api);
		notes_list_api.clear();
	}

	int notes_list_size();
	void set_notes_list_size(int size);

	QVariantList getNotes();

private:
	UserNotes *notesPanel;
	QList<Note_API *> notes_list_api;
	void refreshApi();
};

class Note_API : public ApiObject {
	Q_OBJECT

	Q_PROPERTY(QString name READ getName WRITE setName);
	Q_PROPERTY(QString path READ getPath WRITE setPath);

public:
	explicit Note_API(Note *note) : ApiObject(), note(note) {}
	~Note_API() {}

	QString getName();
	void setName(QString);

	QString getPath();
	void setPath(QString);

private:
	Note *note;
};
} // namespace adiscope

#endif // USER_NOTES_API_HPP
