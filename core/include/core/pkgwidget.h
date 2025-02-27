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

#ifndef PKGWIDGET_H
#define PKGWIDGET_H

#include <QLineEdit>
#include <QWidget>
#include <qboxlayout.h>
#include <menulineedit.h>
#include <menusectionwidget.h>
#include <scopy-core_export.h>
#include <searchbar.h>
#include <pkggridwidget.h>
#include <verticaltabwidget.h>

namespace scopy {

class SCOPY_CORE_EXPORT PkgWidget : public QWidget
{
	Q_OBJECT
public:
	PkgWidget(QWidget *parent = nullptr);
	~PkgWidget();

	QWidget *pkgList(QWidget *parent = nullptr);

public Q_SLOTS:
	void showInstallDialog(const QString &zipPath, const QString &pkgPath);

private Q_SLOTS:
	void onInstall(const QString &zipPath);

Q_SIGNALS:
	void pkgReinstalled(bool installed = true);

private:
	QWidget *createRestartWidget(QWidget *parent);

	QVBoxLayout *m_layScroll;
	QSpacerItem *m_scrollSpacer;
	VerticalTabWidget *m_tabWidget;
};

} // namespace scopy

#endif // PKGWIDGET_H
