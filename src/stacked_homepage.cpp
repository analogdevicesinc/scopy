/*
 * Copyright 2018 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "stacked_homepage.h"

#include <QDebug>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QFileDialog>
#include <QTextBrowser>

using namespace adiscope;
StackedHomepage::StackedHomepage(QWidget *parent) :
	QStackedWidget(parent)
{
	s_hc = new HomepageControls(this);
	this->installEventFilter(s_hc);
	connect(s_hc, &HomepageControls::goLeft, this, &StackedHomepage::moveLeft);
	connect(s_hc, &HomepageControls::goRight, this, &StackedHomepage::moveRight);
	connect(s_hc, &HomepageControls::openFile, this, &StackedHomepage::openFile);

	s_speed = 500;
	s_animationType = QEasingCurve::InOutCubic;
	s_wrap = false;
	s_active = false;
	s_now = QPoint(0, 0);
	s_current = 0;
	s_next = 0;
}

StackedHomepage::~StackedHomepage()
{
	this->removeEventFilter(s_hc);
}

void StackedHomepage::addWidget(QWidget *widget)
{
	QStackedWidget::addWidget(widget);
	s_hc->setVisible(count() > 1);
	s_hc->raise();
}

void StackedHomepage::moveLeft()
{
	slideInPrev();
	s_hc->raise();
}

void StackedHomepage::moveRight()
{
	int currentPage = currentIndex();
	slideInNext();
	s_hc->raise();
}

void StackedHomepage::openFile()
{
	auto export_dialog( new QFileDialog( this ) );
		export_dialog->setWindowModality( Qt::WindowModal );
		export_dialog->setFileMode( QFileDialog::AnyFile );
		export_dialog->setAcceptMode( QFileDialog::AcceptOpen );
		if (export_dialog->exec()){
			QFile f(export_dialog->selectedFiles().at(0));
			QFileInfo f_info(f);
			QTextBrowser *newWindow = new QTextBrowser(this);
			newWindow->setOpenExternalLinks(true);
			newWindow->setFrameShape(QFrame::NoFrame);
			QString path = f_info.absoluteFilePath().remove(f_info.fileName());
			newWindow->setSearchPaths(QStringList(path));
			newWindow->setSource(QUrl(f_info.fileName()));
			addWidget(newWindow);
			slideInNext();
		}
}

void StackedHomepage::setSpeed(int speed)
{
	s_speed = speed;
}

void StackedHomepage::setAnimation(QEasingCurve::Type animationType)
{
	s_animationType = animationType;
}

void StackedHomepage::setWrap(bool wrap)
{
	s_wrap = wrap;
}

void StackedHomepage::slideInNext()
{
	int now = currentIndex();
	if (s_wrap || now < count() - 1)
		slideToIndex(now + 1);
}

void StackedHomepage::slideInPrev()
{
	int now = currentIndex();
	if (s_wrap || now > 0)
		slideToIndex(now - 1);
}

void StackedHomepage::animationDone()
{
	setCurrentIndex(s_next);
	widget(s_current)->hide();
	widget(s_current)->move(s_now);
	s_active = false;
	Q_EMIT animationFinished();
	s_hc->raise();
}

void StackedHomepage::slideToIndex(int index)
{
	s_directions direction;

	if (index > currentIndex()) {
		direction = RIGHT2LEFT;
	} else {
		direction = LEFT2RIGHT;
	}

	if (index > count() - 1) {
		index = 0;
	} else if (index < 0) {
		index = count() - 1;
	}

	slideInWidget(widget(index), direction);
}

void StackedHomepage::slideInWidget(QWidget *newWidget, StackedHomepage::s_directions direction)
{
	if (s_active) {
		return;
	} else {
		s_active = true;
	}

	enum s_directions directionHint;
	int current = currentIndex();
	int next = indexOf(newWidget);
	qDebug() << current << " " << next;
	if (current == next) {
		s_active = false;
		return;
	} else if (current < next) {
		directionHint = RIGHT2LEFT;
	} else {
		directionHint = LEFT2RIGHT;
	}

	int offsetx = frameRect().width();
	int offsety = frameRect().height();
	widget(next)->setGeometry(0, 0, offsetx, offsety);

	if (direction == RIGHT2LEFT) {
		offsetx = - offsetx;
		offsety = 0;
	} else {
		offsety = 0;
	}

	QPoint pnext = widget(next)->pos();
	QPoint pcurrent = widget(current)->pos();
	s_now = pcurrent;

	widget(next)->move(pnext.x() - offsetx, pnext.y() - offsety);
	widget(next)->show();
	widget(next)->raise();

	QPropertyAnimation *animNow = new QPropertyAnimation(widget(current), "pos");
	animNow->setDuration(s_speed);
	animNow->setEasingCurve(s_animationType);
	animNow->setStartValue(QPoint(pcurrent.x(), pcurrent.y()));
	animNow->setEndValue(QPoint(offsetx + pcurrent.x(), offsety + pcurrent.y()));
	QPropertyAnimation *animNext = new QPropertyAnimation(widget(next), "pos");
	animNext->setDuration(s_speed);
	animNext->setEasingCurve(s_animationType);
	animNext->setStartValue(QPoint(-offsetx + pnext.x(), offsety + pnext.y()));
	animNext->setEndValue(QPoint(pnext.x(), pnext.y()));
	QParallelAnimationGroup *animGroup = new QParallelAnimationGroup(this);

	animGroup->addAnimation(animNow);
	animGroup->addAnimation(animNext);

	connect(animGroup, &QParallelAnimationGroup::finished, this, &StackedHomepage::animationDone);

	s_next = next;
	s_current = current;
	s_active = true;
	animGroup->start();

}
