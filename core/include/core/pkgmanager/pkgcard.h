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

#ifndef PKGITEMWIDGET_H
#define PKGITEMWIDGET_H

#include <QLabel>
#include <QWidget>
#include <QComboBox>
#include <QPushButton>
#include <animationpushbutton.h>
#include <toolbuttons.h>
#include <cardwidget.h>
#include <widgets/interactivelabel.h>

namespace scopy {
class PkgCard : public CardWidget
{
	Q_OBJECT
public:
	PkgCard(QWidget *parent = nullptr);
	~PkgCard();

	void setVersions(QStringList versions);
	void installFinished(bool installed);
	void uninstallFinished(bool uninstalled);

	QVariantMap metadata() const;

public Q_SLOTS:
	void fillMetadata(QVariantMap metadata, bool installed = false);

Q_SIGNALS:
	void installClicked();
	void uninstallClicked();
	void preview(const QVariantMap &metadata);
	void hidePreview();
	void categorySelected(const QString &cat);

protected:
	void mousePressEvent(QMouseEvent *event);

private:
	void fillCategories(QStringList categories);

	InstallBtn *m_installBtn;
	UninstallBtn *m_uninstallBtn;
	QVariantMap m_metadata;
	QHBoxLayout *m_categoryLay;

	static QVector<QString> m_categories;
};
} // namespace scopy

#endif // PKGITEMWIDGET_H
