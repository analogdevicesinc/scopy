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

#ifndef SCOPY_WATCHLISTVIEW_H
#define SCOPY_WATCHLISTVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <pluginbase/apiobject.h>
#include "watchlistentry.h"
#include "iiostandarditem.h"

namespace scopy::debugger {
class WatchListView : public QTableWidget
{
	Q_OBJECT
	friend class WatchListView_API;

public:
	explicit WatchListView(QWidget *parent = nullptr);

	void setupUi();
	void connectSignalsAndSlots();

	void saveSettings(QSettings &);
	void loadSettings(QSettings &);

public Q_SLOTS:
	void addToWatchlist(IIOStandardItem *item);
	void removeFromWatchlist(IIOStandardItem *item);
	void currentTreeSelectionChanged(IIOStandardItem *item);
	void refreshWatchlist();

Q_SIGNALS:
	void selectedItem(IIOStandardItem *item);
	void removeItem(IIOStandardItem *item);

protected:
	void resizeEvent(QResizeEvent *event) override;

private:
	QList<int> m_offsets;
	ApiObject *m_apiObject;
	QMap<QString, WatchListEntry *> m_entryObjects;
};

class WatchListView_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(QList<int> offsets READ offsets WRITE setOffsets FINAL)
public:
	explicit WatchListView_API(WatchListView *p);
	~WatchListView_API() = default;

	QList<int> tableHeader() const;
	void setTableHeader(const QList<int> &newTableHeader);

	QList<int> offsets() const;
	void setOffsets(const QList<int> &newOffsets);

private:
	WatchListView *p;
};
} // namespace scopy::debugger

#endif // SCOPY_WATCHLISTVIEW_H
