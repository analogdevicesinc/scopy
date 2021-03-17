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

#include "stacked_homepage.hpp"

#include <QDebug>
#include <QFileDialog>
#include <QParallelAnimationGroup>
#include <QTextBrowser>

#include <scopy/gui/custom_animation.hpp>

using namespace scopy::gui;

StackedHomepage::StackedHomepage(QWidget* parent)
	: QStackedWidget(parent)
{
	m_hc = new HomepageControls(this);
	this->installEventFilter(m_hc);
	connect(m_hc, &HomepageControls::goLeft, this, &StackedHomepage::moveLeft);
	connect(m_hc, &HomepageControls::goRight, this, &StackedHomepage::moveRight);
	connect(m_hc, &HomepageControls::openFile, this, &StackedHomepage::openFile);

	m_speed = 500;
	m_animationType = QEasingCurve::InOutCubic;
	m_wrap = false;
	m_active = false;
	m_now = QPoint(0, 0);
	m_current = 0;
	m_next = 0;
	m_controlsEnabled = true;
}

StackedHomepage::~StackedHomepage() { this->removeEventFilter(m_hc); }

void StackedHomepage::addWidget(QWidget* widget)
{
	QStackedWidget::addWidget(widget);
	m_hc->setVisible(count() > 1);
	m_hc->raise();
	if (currentIndex() < count() - 1) {
		m_hc->enableRight(true);
	}
}

void StackedHomepage::removeWidget(QWidget* widget)
{
	QStackedWidget::removeWidget(widget);
	m_hc->setVisible(count() > 1);
	m_hc->raise();
}

void StackedHomepage::insertWidget(int pos, QWidget* widget)
{
	QStackedWidget::insertWidget(pos, widget);
	m_hc->setVisible(count() > 1);
	m_hc->enableRight(currentIndex() < count() - 1);
	m_hc->enableLeft(currentIndex() > 0);
	m_hc->raise();
}

void StackedHomepage::moveLeft()
{
	if (m_controlsEnabled) {
		slideInPrev();
		m_hc->raise();
	}
	Q_EMIT moved(-1);
}

void StackedHomepage::moveRight()
{
	if (m_controlsEnabled) {
		slideInNext();
		m_hc->raise();
	}
	Q_EMIT moved(1);
}

void StackedHomepage::openFile()
{
	auto export_dialog(new QFileDialog(this));
	export_dialog->setWindowModality(Qt::WindowModal);
	export_dialog->setFileMode(QFileDialog::AnyFile);
	export_dialog->setAcceptMode(QFileDialog::AcceptOpen);
	if (export_dialog->exec()) {
		QFile f(export_dialog->selectedFiles().at(0));
		QFileInfo f_info(f);
		QTextBrowser* newWindow = new QTextBrowser(this);
		newWindow->setOpenExternalLinks(true);
		newWindow->setFrameShape(QFrame::NoFrame);
		QString path = f_info.absoluteFilePath().remove(f_info.fileName());
		newWindow->setSearchPaths(QStringList(path));
		newWindow->setSource(QUrl(f_info.fileName()));
		addWidget(newWindow);
		slideInNext();
	}
}

void StackedHomepage::setSpeed(int speed) { m_speed = speed; }

void StackedHomepage::setAnimation(QEasingCurve::Type animationType) { m_animationType = animationType; }

void StackedHomepage::setWrap(bool wrap) { m_wrap = wrap; }

void StackedHomepage::slideInNext()
{
	int now = currentIndex();
	if (m_wrap || now < count() - 1)
		slideToIndex(now + 1);
}

void StackedHomepage::slideInPrev()
{
	int now = currentIndex();
	if (m_wrap || now > 0)
		slideToIndex(now - 1);
}

void StackedHomepage::animationDone()
{
	setCurrentIndex(m_next);
	widget(m_current)->hide();
	widget(m_current)->move(m_now);
	m_active = false;
	Q_EMIT animationFinished();
	m_hc->raise();
	m_hc->enableLeft(!(currentIndex() == 0));
	m_hc->enableRight(!(currentIndex() == (count() - 1)));
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

void StackedHomepage::slideInWidget(QWidget* newWidget, StackedHomepage::s_directions direction)
{
	if (m_active) {
		if (m_next != indexOf(newWidget)) {
			animationDone();
		} else {
			return;
		}
	}
	m_active = true;

	enum s_directions directionHint;
	int current = currentIndex();
	int next = indexOf(newWidget);
	if (current == next) {
		m_active = false;
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
		offsetx = -offsetx;
		offsety = 0;
	} else {
		offsety = 0;
	}

	QPoint pnext = widget(next)->pos();
	QPoint pcurrent = widget(current)->pos();
	m_now = pcurrent;

	widget(next)->move(pnext.x() - offsetx, pnext.y() - offsety);
	widget(next)->show();
	widget(next)->raise();

	CustomAnimation* animNow = new CustomAnimation(widget(current), "pos");
	animNow->setDuration(m_speed);
	animNow->setEasingCurve(m_animationType);
	animNow->setStartValue(QPoint(pcurrent.x(), pcurrent.y()));
	animNow->setEndValue(QPoint(offsetx + pcurrent.x(), offsety + pcurrent.y()));
	CustomAnimation* animNext = new CustomAnimation(widget(next), "pos");
	animNext->setDuration(m_speed);
	animNext->setEasingCurve(m_animationType);
	animNext->setStartValue(QPoint(-offsetx + pnext.x(), offsety + pnext.y()));
	animNext->setEndValue(QPoint(pnext.x(), pnext.y()));
	QParallelAnimationGroup* animGroup = new QParallelAnimationGroup(this);

	animGroup->addAnimation(animNow);
	animGroup->addAnimation(animNext);

	connect(animGroup, &QParallelAnimationGroup::finished, this, &StackedHomepage::animationDone);

	m_next = next;
	m_current = current;
	m_active = true;
	animGroup->start();
}

bool StackedHomepage::getControlsEnabled() const { return m_controlsEnabled; }

void StackedHomepage::setControlsEnabled(bool value) { m_controlsEnabled = value; }
