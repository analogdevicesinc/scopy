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

#ifndef PKGINSTALLEDTAB_H
#define PKGINSTALLEDTAB_H

#include "pkggridwidget.h"
#include "searchbar.h"
#include <QWidget>
#include <pkgpreviewpage.h>

namespace scopy {
class PkgInstalledTab : public QWidget
{
	Q_OBJECT

public:
	PkgInstalledTab(QWidget *parent = nullptr);
	~PkgInstalledTab();

private Q_SLOTS:
	void onUninstall();
	void previewSwitchClicked(bool checked);
	void onPkgPreview(const QVariantMap &metadata);
	void onCategorySelected(const QString &category);

private:
	void fillPkgSection();
	PkgItemWidget *createPkgItemWidget(const QVariantMap &meta);

	PkgGridWidget *m_pkgGrid;
	SearchBar *m_searchBar;
	PkgPreviewPage *m_preview;
	QSplitter *m_vSplitter;
};
} // namespace scopy

#endif // PKGINSTALLEDTAB_H
