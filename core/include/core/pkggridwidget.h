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
#include <QGridLayout>
#include <QMap>
#include <pkgitemwidget.h>

namespace scopy {

class PkgGridWidget : public QWidget
{
	Q_OBJECT
public:
	PkgGridWidget(QWidget *parent = nullptr);
	~PkgGridWidget();

	void addPkg(PkgItemWidget *pkgItem);
	void removePkg(const QString &pkgName);

	int gridSize() const;
	void updatePkgsStyle(bool property);

public Q_SLOTS:
	void searchPkg(const QStringList &fields, const QStringList &options);

private Q_SLOTS:
	void colNumberChanged(QString pref, QVariant val);

private:
	void hideAll();
	void rebuildLayout();
	void showPkg(QWidget *pkg, int index);
	bool isSearchMatch(const QString &pkgVal, const QStringList &searchingValues);
	bool isFieldMatch(const QVariantMap &pkgMetadata, const QString &field, const QStringList &options);

	QMap<QString, PkgItemWidget *> m_pkgMap;
	QGridLayout *m_layout;
	int m_maxCol;
};
} // namespace scopy

#endif // PKGGRIDWIDGET_H
