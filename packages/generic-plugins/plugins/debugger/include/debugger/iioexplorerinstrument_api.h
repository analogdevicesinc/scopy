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

#ifndef IIOEXPLORERINSTRUMENT_API_H
#define IIOEXPLORERINSTRUMENT_API_H

#include "scopy-debugger_export.h"
#include <pluginbase/apiobject.h>
#include <QList>
#include <QStringList>

namespace scopy::debugger {

class IIOExplorerInstrument;

class SCOPY_DEBUGGER_EXPORT IIOExplorerInstrument_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(QList<int> vSplitter READ vSplitter WRITE setVSplitter FINAL)
	Q_PROPERTY(QList<int> hSplitter READ hSplitter WRITE setHSplitter FINAL)

public:
	explicit IIOExplorerInstrument_API(IIOExplorerInstrument *p);
	~IIOExplorerInstrument_API() = default;

	// === EXISTING PROPERTIES ===
	QList<int> vSplitter() const;
	void setVSplitter(const QList<int> &newSplitter);
	QList<int> hSplitter() const;
	void setHSplitter(const QList<int> &newSplitter);

	// === TAB CONTROL (Main tabs) ===
	// Tab indices: 0=IIO Attributes, 1=Log, 2=Code Generator
	Q_INVOKABLE void setCurrentTab(int index);
	Q_INVOKABLE int getCurrentTab();
	Q_INVOKABLE void showIIOAttributesTab();
	Q_INVOKABLE void showLogTab();
	Q_INVOKABLE void showCodeGeneratorTab();

	// === TREE NAVIGATION ===
	// Select an item by path (e.g., "context0/ad9361-phy/voltage0/hardwaregain")
	Q_INVOKABLE bool selectItemByPath(const QString &path);
	Q_INVOKABLE QString getSelectedItemPath();
	Q_INVOKABLE QString getSelectedItemName();
	Q_INVOKABLE QString getSelectedItemType();

	// === TREE EXPAND/COLLAPSE ===
	Q_INVOKABLE bool expandItem(const QString &path);
	Q_INVOKABLE bool collapseItem(const QString &path);
	Q_INVOKABLE bool isItemExpanded(const QString &path);
	Q_INVOKABLE int getChildCount(const QString &path);
	Q_INVOKABLE QStringList getChildNames(const QString &path);

	// === WATCHLIST ===
	Q_INVOKABLE bool addCurrentItemToWatchlist();
	Q_INVOKABLE bool addItemToWatchlistByPath(const QString &path);
	Q_INVOKABLE bool removeItemFromWatchlistByPath(const QString &path);
	Q_INVOKABLE QStringList getWatchlistPaths();
	Q_INVOKABLE void clearWatchlist();
	Q_INVOKABLE void refreshWatchlist();
	Q_INVOKABLE bool writeWatchlistAttributeValue(const QString &path, const QString &value);

	// === ATTRIBUTE VALUES ===
	Q_INVOKABLE QString readAttributeValue(const QString &path);
	Q_INVOKABLE bool writeAttributeValue(const QString &path, const QString &value);
	Q_INVOKABLE void triggerRead();

	// === CODE GENERATOR ===
	Q_INVOKABLE QString getGeneratedCode();
	Q_INVOKABLE bool saveGeneratedCode(const QString &filePath);

	// === LOG ===
	Q_INVOKABLE QString getLogContent();
	Q_INVOKABLE void clearLog();

	// === SEARCH/FILTER ===
	Q_INVOKABLE void setSearchText(const QString &text);
	Q_INVOKABLE QString getSearchText();
	Q_INVOKABLE QString getSearchPlaceholderText();
	Q_INVOKABLE int getVisibleItemCount();

	// === DETAILS VIEW TAB ===
	// Tab indices: 0=GUI View, 1=IIO View, 2=Generated Code
	Q_INVOKABLE void setDetailsViewTab(int index);
	Q_INVOKABLE int getDetailsViewTab();

private:
	IIOExplorerInstrument *p;
};

} // namespace scopy::debugger

#endif // IIOEXPLORERINSTRUMENT_API_H
