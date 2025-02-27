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

#ifndef PKGZIPWIDGET_H
#define PKGZIPWIDGET_H

#include <QWidget>
#include <filebrowserwidget.h>
#include <infoheaderwidget.h>
#include <menucollapsesection.h>
#include <pkgpreviewpage.h>

namespace scopy {
class PkgZipWidget : public QWidget
{
	Q_OBJECT
public:
	PkgZipWidget(QWidget *parent);
	~PkgZipWidget();

public Q_SLOTS:
	void onZipMetadata(QVariantMap metadata);
	void onInstallFinished();

Q_SIGNALS:
	void pkgPathSelected(const QString &path);
	void installClicked(const QString &zipPath);

private Q_SLOTS:
	void fileBrowserEvent(const QString &text);

private:
	MenuSectionCollapseWidget *createSection(QString title, QString description, QWidget *parent = nullptr);
	InfoHeaderWidget *getInfoHeader(MenuCollapseSection *section);
	MenuSectionCollapseWidget *createBrowserSection(QWidget *parent = nullptr);

	FileBrowserWidget *m_fileBrowser;
	PkgPreviewPage *m_preview;
	QLabel *m_warningLabel;
	InstallBtn *m_installBtn;
};
} // namespace scopy

#endif // PKGZIPWIDGET_H
