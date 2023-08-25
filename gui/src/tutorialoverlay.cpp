#include "tutorialoverlay.h"
#include <QLoggingCategory>
#include <QResizeEvent>
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
	// the event filter should be installed only after the UI is built
	this->parent->installEventFilter(this);
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
		ui->btnNext->setText(tr("Finish"));
}

void TutorialOverlay::finish() {
	qInfo(CAT_TUTORIALOVERLAY)<<"Tutorial Finished";
	overlay->deleteLater();
	deleteLater();

	Q_EMIT finished();
}

void TutorialOverlay::abort() {
	qInfo(CAT_TUTORIALOVERLAY) << "Tutorial Aborted";
	if(!highlights.isEmpty()) {
		// delete previous highlight
		for (TintedOverlay *highlight : qAsConst(highlights)) {
			highlight->deleteLater();
		}
		highlights.clear();
	}
	overlay->deleteLater();
	deleteLater();

	Q_EMIT aborted();
}

void TutorialOverlay::buildUi()
{
	qDebug(CAT_TUTORIALOVERLAY)<<"build";
	overlay = new TintedOverlay(parent);

	ui = new Ui::Tutorial();
	ui->setupUi(this);
	ui->description->setStyleSheet("background-color: rgba(0,0,0,60);color: white;");
	ui->title->setStyleSheet("color:white");
	ui->btnExit->setText(tr(ui->btnExit->text().toStdString().c_str()));
	ui->btnExit->setStyleSheet("width:80;height:20");
	ui->btnExit->setProperty("blue_button", true);
	ui->btnNext->setText(tr(ui->btnNext->text().toStdString().c_str()));
	ui->btnNext->setStyleSheet("width:80;height:20");
	ui->btnNext->setProperty("blue_button", true);
	ui->btnNext->setFocus();
	connect(ui->btnNext,&QPushButton::clicked,this,&TutorialOverlay::next);
	connect(ui->btnExit, &QPushButton::clicked, this, &TutorialOverlay::abort);

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

bool TutorialOverlay::eventFilter(QObject *watched, QEvent *event) {
	if (watched == this->parent && event->type() == QEvent::Resize) {
		QWidget* w = qobject_cast<QWidget*>(watched);
		if (w) {
			QSize s = w->size();
			this->overlay->resize(s);
			this->move((overlay->width() - this->width()) / 2, (overlay->height() - this->height()) / 2);
		}
	}

	return QObject::eventFilter(watched, event);
}

#include "moc_tutorialoverlay.cpp"
