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

#ifndef TUTORIALOVERLAY_H
#define TUTORIALOVERLAY_H

#include "scopy-gui_export.h"
#include "tintedoverlay.h"
#include "tutorialchapter.h"
#include "widgets/popupwidget.h"

#include <QWidget>

#include <widgets/hoverwidget.h>

namespace Ui {
class Tutorial;
}

namespace scopy::gui {

class SCOPY_GUI_EXPORT TutorialOverlay : public QWidget
{
	Q_OBJECT
public:
	explicit TutorialOverlay(QWidget *parent = nullptr);
	~TutorialOverlay();

	TutorialChapter *addChapter(const QList<QWidget *> &subjects, const QString &description,
				    QWidget *mainWidget = nullptr, int x_offset = 0, int y_offset = 0,
				    HoverPosition anchor = HoverPosition::HP_CENTER,
				    HoverPosition content = HoverPosition::HP_CENTER);
	TutorialChapter *addChapter(QWidget *subject, QString description);
	void addChapter(TutorialChapter *ch);

	const QString &getTitle() const;
	void setTitle(const QString &newTitle);

public Q_SLOTS:
	virtual void start();
	void next();
	void finish();
	void abort();

Q_SIGNALS:
	/**
	 * @brief Used to signal that the current tutorial was fully completed by the user, not exited early.
	 * */
	void finished();

	/**
	 * @brief Used to signal that the current tutorial was exited early, without completion, by the user.
	 * */
	void aborted();

private:
	void buildUi();
	void initPopupWidget();
	void cleanupChapter();
	bool eventFilter(QObject *watched, QEvent *event) override;

	QList<TutorialChapter *> chapter;
	QWidget *parent;
	TintedOverlay *overlay;
	QList<TintedOverlay *> highlights;
	HoverWidget *m_hoverWidget;
	QString title;
	int cnt;
	PopupWidget *m_popupWidget;
};
} // namespace scopy::gui

#endif // TUTORIALOVERLAY_H
