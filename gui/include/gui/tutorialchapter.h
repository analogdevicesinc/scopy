#ifndef TUTORIALCHAPTER_H
#define TUTORIALCHAPTER_H

#include <QWidget>
#include <QList>
#include <scopy-gui_export.h>

namespace scopy::gui {
class SCOPY_GUI_EXPORT TutorialChapter : public QObject {
	Q_OBJECT
public:
	TutorialChapter(QObject *parent = nullptr);
	~TutorialChapter();

	static TutorialChapter* build(QList<QWidget *> subjects, QString description, QObject *parent = nullptr);

	void addSubject( QWidget* s);

	QList<QWidget*> subjects;
	QString description;

	const QString &getDescription() const;
	void setDescription(const QString &newDescription);

Q_SIGNALS:
	void chapterStarted();
	void chapterFinished();
};
}
#endif // TUTORIALCHAPTER_H
