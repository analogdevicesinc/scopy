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

#ifndef PKGGRIDWIDGET_H
#define PKGGRIDWIDGET_H

#include <QWidget>
#include <pkgcard.h>
#include <gridwidget.h>

namespace scopy {

class PkgGridWidget : public GridWidget
{
	Q_OBJECT
public:
	PkgGridWidget(QWidget *parent = nullptr);
	~PkgGridWidget();

	void searchPkg(const QStringList &fields, const QStringList &options);
	void updatePkgsStyle(bool property);

private:
	bool isFieldMatch(const QVariantMap &pkgMetadata, const QString &field, const QStringList &options);
	bool isSearchMatch(const QString &pkgVal, const QStringList &searchingValues);
};
} // namespace scopy

#endif // PKGGRIDWIDGET_H
