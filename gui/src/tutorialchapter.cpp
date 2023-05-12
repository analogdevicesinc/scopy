#include "tutorialchapter.h"

using namespace scopy::gui;

TutorialChapter::TutorialChapter(QObject *parent) : QObject(parent)
{

}

TutorialChapter::~TutorialChapter()
{

}

TutorialChapter *TutorialChapter::build(QList<QWidget *> subjects, QString description, QObject *parent)
{
	TutorialChapter *ch = new TutorialChapter(parent);
	for(QWidget *subject : subjects) {
		ch->addSubject(subject);
	}
	ch->setDescription(description);
	return ch;
}

void TutorialChapter::addSubject(QWidget *s)
{
	subjects.append(s);
}

const QString &TutorialChapter::getDescription() const
{
	return description;
}

void TutorialChapter::setDescription(const QString &newDescription)
{
	description = newDescription;
}
