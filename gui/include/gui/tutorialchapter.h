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

#ifndef TUTORIALCHAPTER_H
#define TUTORIALCHAPTER_H

#include <QList>
#include <QWidget>

#include <scopy-gui_export.h>
#include <widgets/hoverwidget.h>

namespace scopy::gui {
class SCOPY_GUI_EXPORT TutorialChapter : public QObject
{
	Q_OBJECT
public:
	TutorialChapter(QObject *parent = nullptr);
	~TutorialChapter();

	static TutorialChapter *build(const QList<QWidget *> &subjects, const QString &description,
				      QWidget *mainSubject, int x_offset, int y_offset, HoverPosition anchor,
				      HoverPosition content, QObject *parent = nullptr);

	void addSubject(QWidget *s);

	QList<QWidget *> subjects;
	QWidget *mainSubject; // the main subject is the widget that will act as the anchor for the overlay
	QString description;
	int x_offset;
	int y_offset;
	HoverPosition anchor;
	HoverPosition content;

	int getXOffset() const;
	void setXOffset(int xOffset);

	int getYOffset() const;
	void setYOffset(int yOffset);

	HoverPosition getAnchor() const;
	void setAnchor(HoverPosition anchor);

	HoverPosition getContent() const;
	void setContent(HoverPosition content);

	const QString &getDescription() const;
	void setDescription(const QString &newDescription);

	QWidget *getMainSubject() const;
	void setMainSubject(QWidget *mainSubject);

Q_SIGNALS:
	void chapterStarted();
	void chapterFinished();
};
} // namespace scopy::gui
#endif // TUTORIALCHAPTER_H
