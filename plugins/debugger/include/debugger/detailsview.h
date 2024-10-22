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

#ifndef SCOPY_DETAILSVIEW_H
#define SCOPY_DETAILSVIEW_H

#include <QWidget>
#include <QTabWidget>
#include <QTextBrowser>
#include "iiostandarditem.h"
#include "guidetailsview.h"
#include "clidetailsview.h"
#include "pathtitle.h"

namespace scopy::debugger {
class DetailsView : public QWidget
{
	Q_OBJECT
public:
	explicit DetailsView(QWidget *parent = nullptr);
	void setIIOStandardItem(IIOStandardItem *item);
	void refreshIIOView();

	QPushButton *readBtn();
	QPushButton *addToWatchlistBtn();

	// add:true = +, add:false = X
	void setAddToWatchlistState(bool add);

Q_SIGNALS:
	void pathSelected(QString path);

private:
	void setupUi();

	IIOStandardItem *m_currentItem;
	GuiDetailsView *m_guiDetailsView;
	CliDetailsView *m_cliDetailsView;
	QTabWidget *m_tabWidget;
	QWidget *m_guiView;
	QWidget *m_iioView;
	QWidget *m_titleContainer;
	PathTitle *m_titlePath;
	QPushButton *m_readBtn;
	QPushButton *m_addToWatchlistBtn;
};
} // namespace scopy::debugger

#endif // SCOPY_DETAILSVIEW_H
