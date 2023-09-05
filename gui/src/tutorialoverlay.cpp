/*
 * Copyright (c) 2023 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */


#include "tutorialoverlay.h"
#include <QLoggingCategory>
#include <QResizeEvent>
#include <utility>
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

TutorialChapter* TutorialOverlay::addChapter(const QList<QWidget *>& subjects,
					     const QString& description,
					     QWidget* mainWidget,
					     int x_offset,
					     int y_offset,
					     HoverPosition anchor,
					     HoverPosition content)
{
	// in case TutorialOverlay is used without TutorialBuilder
	if (!mainWidget) {
		mainWidget = subjects[0];
	}
	TutorialChapter* ch = TutorialChapter::build(subjects, description,
						     mainWidget,
						     x_offset, y_offset,
						     anchor, content, this);
	chapter.append(ch);
	ch->setMainSubject(mainWidget);
	return ch;
}

TutorialChapter *TutorialOverlay::addChapter(QWidget *subject, QString description)
{
	QList<QWidget*> list;

	// pass empty list if subject is nullptr
	if(subject != nullptr)
		list.append(subject);

	return addChapter(list, description, subject); // if there is only one widget, it will be the main widget
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
		m_hoverWidget->deleteLater();
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
		m_hoverWidget = new HoverWidget(this, chapter[cnt]->mainSubject, qApp->activeWindow());

		m_hoverWidget->setContentPos(chapter[cnt]->content);
		m_hoverWidget->setAnchorPos(chapter[cnt]->anchor);
		m_hoverWidget->setAnchorOffset(QPoint(chapter[cnt]->x_offset, chapter[cnt]->y_offset));

		m_hoverWidget->raise();
		m_hoverWidget->setVisible(true);
		for (QWidget *subj : qAsConst(subjects)) {
			subj->raise();
			ui->btnNext->setFocus();
			auto* highlight = new TintedOverlay(subj,QColor(255,255,255,35));
			highlight->raise();
			highlight->show();
			highlights.append(highlight);
		}
	}

	QString storyTitle = title + QString::number(cnt + 1) + "/" + QString::number(chapter.count());
	ui->title->setText(storyTitle);

	int pw = parent->geometry().width();
	int ph = parent->geometry().height();
	int w = 400;
	int h = 150;

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
	m_hoverWidget->deleteLater();
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
		m_hoverWidget->deleteLater();
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
	ui->btnExit->setStyleSheet("width:80;height:20");
	ui->btnExit->setProperty("blue_button", true);
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
		}
	}

	return QObject::eventFilter(watched, event);
}

#include "moc_tutorialoverlay.cpp"
