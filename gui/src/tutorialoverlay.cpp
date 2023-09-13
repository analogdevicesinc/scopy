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
	cleanupChapter();

	if(cnt == chapter.count()) {
		// stop story
		finish();
		return;
	}
	if(cnt > 0)
		Q_EMIT chapter[cnt - 1]->chapterFinished();

	Q_EMIT chapter[cnt]->chapterStarted();

	initPopupWidget();
	m_popupWidget->setDescription(chapter[cnt]->description);
	overlay->raise();
	overlay->clearHoles();

	QList<QWidget*> subjects = chapter[cnt]->subjects;
	if(!subjects.isEmpty()) {
		overlay->setHoles(subjects);

		m_hoverWidget = new HoverWidget(m_popupWidget, chapter[cnt]->mainSubject, qApp->activeWindow());
		m_hoverWidget->setContentPos(chapter[cnt]->content);
		m_hoverWidget->setAnchorPos(chapter[cnt]->anchor);
		m_hoverWidget->setAnchorOffset(QPoint(chapter[cnt]->x_offset, chapter[cnt]->y_offset));

		m_hoverWidget->raise();
		m_hoverWidget->setVisible(true);
		for (QWidget *subj : qAsConst(subjects)) {
			subj->raise();
			m_popupWidget->setFocusOnContinueButton();
			auto* highlight = new TintedOverlay(subj,QColor(255,255,255,35));
			highlight->raise();
			highlight->show();
			highlights.append(highlight);
		}
	}

	QString storyTitle = title + " " + QString::number(cnt + 1) + "/" + QString::number(chapter.count());
	m_popupWidget->setTitle(storyTitle);

	raise();
	show();
	m_popupWidget->raise();

	cnt++;
	if(cnt == chapter.count())
		m_popupWidget->setContinueButtonText("Finish");
}

void TutorialOverlay::finish() {
	qInfo(CAT_TUTORIALOVERLAY)<<"Tutorial Finished";
	overlay->deleteLater();
	cleanupChapter();

	Q_EMIT finished();
}

void TutorialOverlay::abort() {
	qInfo(CAT_TUTORIALOVERLAY) << "Tutorial Aborted";
	cleanupChapter();
	overlay->deleteLater();
	deleteLater();

	Q_EMIT aborted();
}

void TutorialOverlay::buildUi()
{
	qDebug(CAT_TUTORIALOVERLAY)<<"build";
	overlay = new TintedOverlay(parent);

	initPopupWidget();

	overlay->raise();
	overlay->show();
}

void TutorialOverlay::initPopupWidget() {
	m_popupWidget = new PopupWidget();
	m_popupWidget->setFocusOnContinueButton();
	connect(m_popupWidget, &PopupWidget::continueButtonClicked, this, &TutorialOverlay::next);
	connect(m_popupWidget, &PopupWidget::exitButtonClicked, this, &TutorialOverlay::abort);
}

void TutorialOverlay::cleanupChapter() {
	if(!highlights.isEmpty()) {
		// delete previous highlight
		for (TintedOverlay *highlight : qAsConst(highlights)) {
			delete highlight;
		}
		highlights.clear();

		delete m_popupWidget;
		delete m_hoverWidget;
	}
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
