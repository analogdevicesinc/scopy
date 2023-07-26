#include "tutorialoverlay.h"
#include <QLoggingCategory>
#include "ui_tutorial.h"

Q_LOGGING_CATEGORY(CAT_TUTORIALOVERLAY,"TutorialOverlay");
using namespace scopy::gui;
TutorialOverlay::TutorialOverlay(QWidget *parent)
    : QWidget(parent), parent(parent)
{
	qDebug(CAT_TUTORIALOVERLAY)<<"ctor";
	overlay = nullptr;
}

TutorialOverlay::~TutorialOverlay()
{
	qDebug(CAT_TUTORIALOVERLAY)<<"dtor";
}

TutorialChapter* TutorialOverlay::addChapter(QList<QWidget *> subjects, QString description)
{
	TutorialChapter* ch = TutorialChapter::build(subjects,description,this);
	chapter.append(ch);
	return ch;
}

TutorialChapter *TutorialOverlay::addChapter(QWidget *subject, QString description)
{

	QList<QWidget*> list;

	// pass empty list if subject is nullptr
	if(subject != nullptr)
		list.append(subject);

	return addChapter(list,description);
}

void TutorialOverlay::addChapter(TutorialChapter *ch)
{
	chapter.append(ch);
}

void TutorialOverlay::start()
{
	qInfo(CAT_TUTORIALOVERLAY)<<"Tutorial started";
	cnt = 0;

	buildUi();
	next();
}

void TutorialOverlay::next()
{
	if(!highlights.isEmpty()) {
		// delete previous highlight
		for (TintedOverlay *highlight : qAsConst(highlights)) {
			highlight->deleteLater();
		}
		highlights.clear();
	}

	if(cnt == chapter.count()) {
		// stop story
		finish();
		return;
	}
	if(cnt > 0)
		Q_EMIT chapter[cnt - 1]->chapterFinished();

	Q_EMIT chapter[cnt]->chapterStarted();

	ui->description->setMarkdown(chapter[cnt]->description);
	overlay->raise();
	overlay->clearHoles();

	QList<QWidget*> subjects = chapter[cnt]->subjects;
	if(!subjects.isEmpty()) {
		overlay->setHoles(subjects);
		for (QWidget *subj : qAsConst(subjects)) {
			subj->raise();
			TintedOverlay* highlight = new TintedOverlay(subj,QColor(255,255,255,35));
			highlight->raise();
			highlight->show();
			highlights.append(highlight);
		}
	}

	QString storyTitle = title + QString::number(cnt + 1) + "/" +QString::number(chapter.count());
	ui->title->setText(storyTitle);

	int pw = parent->geometry().width();
	int ph = parent->geometry().height();
	int w = 600;
	int h = 400;

	setGeometry((pw-w)/2,(ph-h)/2,w,h);

	raise();
	show();

	cnt++;
	if(cnt == chapter.count())
		ui->btnNext->setText("Finish");
}

void TutorialOverlay::finish() {
	qInfo(CAT_TUTORIALOVERLAY)<<"Tutorial Finished";
	overlay->deleteLater();
	deleteLater();

	Q_EMIT finished();
}

void TutorialOverlay::buildUi()
{
	qDebug(CAT_TUTORIALOVERLAY)<<"build";
	overlay = new TintedOverlay(parent);

	ui = new Ui::Tutorial();
	ui->setupUi(this);
	ui->description->setStyleSheet("background-color: rgba(0,0,0,60);color: white;");
	ui->title->setStyleSheet("color:white");
	ui->btnNext->setStyleSheet("width:80;height:20");
	ui->btnNext->setProperty("blue_button", true);
	connect(ui->btnNext,&QPushButton::clicked,this,&TutorialOverlay::next);

	overlay->raise();
	overlay->show();

}

const QString &TutorialOverlay::getTitle() const
{
	return title;
}

void TutorialOverlay::setTitle(const QString &newTitle)
{
	title = newTitle;
}
