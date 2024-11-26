/*
 * Copyright (c) 2024 Analog Devices Inc.
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
 *
 */

#ifndef WHATSNEWOVERLAY_H
#define WHATSNEWOVERLAY_H

#include <QLabel>
#include <QWidget>
#include <popupwidget.h>
#include "scopy-core_export.h"
#include <QButtonGroup>
#include <QComboBox>
#include <QStackedWidget>

namespace scopy {
class SCOPY_CORE_EXPORT WhatsNewOverlay : public QWidget
{
	Q_OBJECT
public:
	explicit WhatsNewOverlay(QWidget *parent = nullptr);
	~WhatsNewOverlay();

	void showOverlay();
	void enableTintedOverlay(bool enable = true);

private:
	PopupWidget *m_popupWidget;
	QString getHtmlPageContent(QString fileName);
	void initCarousel();
	void generateVersionPage(QString filePath);

	QStackedWidget *m_carouselWidget;
	QComboBox *m_versionCb;
	gui::TintedOverlay *m_tintedOverlay;
};
} // namespace scopy
#endif // WHATSNEWOVERLAY_H
