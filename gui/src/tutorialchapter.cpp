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


#include "tutorialchapter.h"
#include <QLoggingCategory>

using namespace scopy::gui;

Q_LOGGING_CATEGORY(CAT_TUTORIALCHAPTER, "TutorialChapter")

TutorialChapter::TutorialChapter(QObject *parent) : QObject(parent)
{

}

TutorialChapter::~TutorialChapter()
{

}

TutorialChapter *TutorialChapter::build(const QList<QWidget *>& subjects,
					const QString& description,
					QWidget* mainSubject,
					int x_offset,
					int y_offset,
					HoverPosition anchor,
					HoverPosition content, QObject *parent)
{
	TutorialChapter *ch = new TutorialChapter(parent);
	for(QWidget *subject : subjects) {
		ch->addSubject(subject);
	}
	ch->setDescription(description);
	ch->setMainSubject(mainSubject);
	ch->setXOffset(x_offset);
	ch->setYOffset(y_offset);
	ch->setAnchor(anchor);
	ch->setContent(content);
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

QWidget *TutorialChapter::getMainSubject() const {
	return mainSubject;
}

void TutorialChapter::setMainSubject(QWidget *newMainSubject) {
	if (!subjects.contains(newMainSubject)) {
		qWarning(CAT_TUTORIALCHAPTER) << "The new main subject is not part of the subject list.";
	}
	this->mainSubject = newMainSubject;
}

int TutorialChapter::getXOffset() const {
	return x_offset;
}

void TutorialChapter::setXOffset(int xOffset) {
	x_offset = xOffset;
}

int TutorialChapter::getYOffset() const {
	return y_offset;
}

void TutorialChapter::setYOffset(int yOffset) {
	y_offset = yOffset;
}

scopy::HoverPosition TutorialChapter::getAnchor() const {
	return anchor;
}

void TutorialChapter::setAnchor(scopy::HoverPosition newAnchor) {
	anchor = newAnchor;
}

scopy::HoverPosition TutorialChapter::getContent() const {
	return content;
}

void TutorialChapter::setContent(scopy::HoverPosition newContent) {
	content = newContent;
}
