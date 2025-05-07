/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef PKGPREVIEWPAGE_H
#define PKGPREVIEWPAGE_H

#include "infopage.h"
#include <QWidget>
#include <infoheaderwidget.h>
#include <menusectionwidget.h>

namespace scopy {

class PkgPreviewPage : public QFrame
{
	Q_OBJECT

public:
	PkgPreviewPage(QWidget *parent = nullptr);
	~PkgPreviewPage();

	void updatePreview(const QVariantMap &metadata);

private:
	InfoHeaderWidget *getInfoHeader(MenuCollapseSection *section);
	MenuSectionCollapseWidget *createSection(QString title, QString description, QWidget *parent);

	InfoPage *m_infoPage;
};

} // namespace scopy

#endif // PKGPREVIEWPAGE_H
