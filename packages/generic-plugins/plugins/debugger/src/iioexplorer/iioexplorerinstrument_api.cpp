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

#include "iioexplorerinstrument_api.h"
#include "iioexplorerinstrument.h"
#include "iiodebuglogger.h"
#include "detailsview.h"
#include "codegenerator.h"
#include "debuggerloggingcategories.h"
#include "iiomodel.h"
#include "iiostandarditem.h"
#include "watchlistview.h"
#include "watchlistentry.h"

#include <QFile>
#include <QSplitter>
#include <QTabWidget>
#include <QTextBrowser>
#include <QLineEdit>
#include <gui/widgets/searchbar.h>

using namespace scopy::debugger;

IIOExplorerInstrument_API::IIOExplorerInstrument_API(IIOExplorerInstrument *p)
	: ApiObject(p)
	, p(p)
{}

// ============================================
// EXISTING PROPERTIES (moved from iioexplorerinstrument.cpp)
// ============================================

QList<int> IIOExplorerInstrument_API::vSplitter() const { return p->m_VSplitter->sizes(); }

void IIOExplorerInstrument_API::setVSplitter(const QList<int> &newSplitter)
{
	if(newSplitter.size() != 2) {
		qWarning(CAT_DEBUGGERIIOMODEL) << "Invalid splitter size. Expected 2, got" << newSplitter.size();
		return;
	}

	if(newSplitter[0] <= 0 || newSplitter[1] <= 0) {
		qWarning(CAT_DEBUGGERIIOMODEL) << "Invalid splitter value. Resetting to default.";
		int sum = newSplitter[0] + newSplitter[1];
		p->m_VSplitter->setSizes(QList<int>() << (sum * 2) / 3 << sum / 3);
		return;
	}

	p->m_VSplitter->setSizes(newSplitter);
}

QList<int> IIOExplorerInstrument_API::hSplitter() const { return p->m_HSplitter->sizes(); }

void IIOExplorerInstrument_API::setHSplitter(const QList<int> &newSplitter)
{
	if(newSplitter.size() != 2) {
		qWarning(CAT_DEBUGGERIIOMODEL) << "Invalid splitter size. Expected 2, got" << newSplitter.size();
		return;
	}

	if(newSplitter[0] <= 0 || newSplitter[1] <= 0) {
		qWarning(CAT_DEBUGGERIIOMODEL) << "Invalid splitter value. Resetting to default.";
		int sum = newSplitter[0] + newSplitter[1];
		p->m_HSplitter->setSizes(QList<int>() << sum / 4 << (sum * 3) / 4);
		return;
	}

	p->m_HSplitter->setSizes(newSplitter);
}

// ============================================
// TAB CONTROL
// ============================================

void IIOExplorerInstrument_API::setCurrentTab(int index)
{
	if(index >= 0 && index < p->m_tabWidget->count()) {
		p->m_tabWidget->setCurrentIndex(index);
	}
}

int IIOExplorerInstrument_API::getCurrentTab() { return p->m_tabWidget->currentIndex(); }

void IIOExplorerInstrument_API::showIIOAttributesTab() { setCurrentTab(0); }

void IIOExplorerInstrument_API::showLogTab() { setCurrentTab(1); }

void IIOExplorerInstrument_API::showCodeGeneratorTab() { setCurrentTab(2); }

// ============================================
// TREE NAVIGATION
// ============================================

bool IIOExplorerInstrument_API::selectItemByPath(const QString &path)
{
	QStringList pathList = path.split('/', Qt::SkipEmptyParts);
	QStandardItem *root = p->m_iioModel->getModel()->invisibleRootItem()->child(0);
	IIOStandardItem *iioRoot = dynamic_cast<IIOStandardItem *>(root);
	if(!iioRoot) {
		return false;
	}

	IIOStandardItem *item = p->findItemByPath(iioRoot, pathList);
	if(!item) {
		return false;
	}

	p->selectItem(item);
	return true;
}

QString IIOExplorerInstrument_API::getSelectedItemPath()
{
	if(!p->m_currentlySelectedItem) {
		return QString();
	}
	return p->m_currentlySelectedItem->path();
}

QString IIOExplorerInstrument_API::getSelectedItemName()
{
	if(!p->m_currentlySelectedItem) {
		return QString();
	}
	return p->m_currentlySelectedItem->name();
}

QString IIOExplorerInstrument_API::getSelectedItemType()
{
	if(!p->m_currentlySelectedItem) {
		return QString();
	}
	return p->m_currentlySelectedItem->typeString();
}

// ============================================
// TREE EXPAND/COLLAPSE
// ============================================

bool IIOExplorerInstrument_API::expandItem(const QString &path)
{
	QStringList pathList = path.split('/', Qt::SkipEmptyParts);
	QStandardItem *root = p->m_iioModel->getModel()->invisibleRootItem()->child(0);
	IIOStandardItem *iioRoot = dynamic_cast<IIOStandardItem *>(root);
	if(!iioRoot) {
		return false;
	}

	IIOStandardItem *item = p->findItemByPath(iioRoot, pathList);
	if(!item) {
		return false;
	}

	// Cast to QStandardItem* to call QStandardItem::index() instead of IIOStandardItem::index()
	QModelIndex proxyIndex = p->m_proxyModel->mapFromSource(static_cast<QStandardItem *>(item)->index());
	if(!proxyIndex.isValid()) {
		return false;
	}

	p->m_treeView->expand(proxyIndex);
	return true;
}

bool IIOExplorerInstrument_API::collapseItem(const QString &path)
{
	QStringList pathList = path.split('/', Qt::SkipEmptyParts);
	QStandardItem *root = p->m_iioModel->getModel()->invisibleRootItem()->child(0);
	IIOStandardItem *iioRoot = dynamic_cast<IIOStandardItem *>(root);
	if(!iioRoot) {
		return false;
	}

	IIOStandardItem *item = p->findItemByPath(iioRoot, pathList);
	if(!item) {
		return false;
	}

	// Cast to QStandardItem* to call QStandardItem::index() instead of IIOStandardItem::index()
	QModelIndex proxyIndex = p->m_proxyModel->mapFromSource(static_cast<QStandardItem *>(item)->index());
	if(!proxyIndex.isValid()) {
		return false;
	}

	p->m_treeView->collapse(proxyIndex);
	return true;
}

bool IIOExplorerInstrument_API::isItemExpanded(const QString &path)
{
	QStringList pathList = path.split('/', Qt::SkipEmptyParts);
	QStandardItem *root = p->m_iioModel->getModel()->invisibleRootItem()->child(0);
	IIOStandardItem *iioRoot = dynamic_cast<IIOStandardItem *>(root);
	if(!iioRoot) {
		return false;
	}

	IIOStandardItem *item = p->findItemByPath(iioRoot, pathList);
	if(!item) {
		return false;
	}

	// Cast to QStandardItem* to call QStandardItem::index() instead of IIOStandardItem::index()
	QModelIndex proxyIndex = p->m_proxyModel->mapFromSource(static_cast<QStandardItem *>(item)->index());
	if(!proxyIndex.isValid()) {
		return false;
	}

	return p->m_treeView->isExpanded(proxyIndex);
}

int IIOExplorerInstrument_API::getChildCount(const QString &path)
{
	QStringList pathList = path.split('/', Qt::SkipEmptyParts);
	QStandardItem *root = p->m_iioModel->getModel()->invisibleRootItem()->child(0);
	IIOStandardItem *iioRoot = dynamic_cast<IIOStandardItem *>(root);
	if(!iioRoot) {
		return 0;
	}

	IIOStandardItem *item = p->findItemByPath(iioRoot, pathList);
	if(!item) {
		return 0;
	}

	return item->rowCount();
}

QStringList IIOExplorerInstrument_API::getChildNames(const QString &path)
{
	QStringList result;
	QStringList pathList = path.split('/', Qt::SkipEmptyParts);
	QStandardItem *root = p->m_iioModel->getModel()->invisibleRootItem()->child(0);
	IIOStandardItem *iioRoot = dynamic_cast<IIOStandardItem *>(root);
	if(!iioRoot) {
		return result;
	}

	IIOStandardItem *item = p->findItemByPath(iioRoot, pathList);
	if(!item) {
		return result;
	}

	for(int i = 0; i < item->rowCount(); ++i) {
		IIOStandardItem *child = dynamic_cast<IIOStandardItem *>(item->child(i));
		if(child) {
			result.append(child->name());
		}
	}

	return result;
}

// ============================================
// WATCHLIST
// ============================================

bool IIOExplorerInstrument_API::addCurrentItemToWatchlist()
{
	if(!p->m_currentlySelectedItem) {
		return false;
	}
	if(p->m_currentlySelectedItem->isWatched()) {
		return true;
	}

	p->m_currentlySelectedItem->setWatched(true);
	p->m_watchListView->addToWatchlist(p->m_currentlySelectedItem);
	p->m_detailsView->setAddToWatchlistState(false);

	// Update code generator
	QList<CodeGenerator::CodeGeneratorRecipe> recipes;
	for(auto item : *p->m_watchListView->watchListEntries()) {
		recipes.append(CodeGenerator::convertToCodeGeneratorRecipe(item->item(), p->m_uri));
	}
	p->m_generatedCodeBrowser->setPlainText(CodeGenerator::generateCode(recipes));

	return true;
}

bool IIOExplorerInstrument_API::addItemToWatchlistByPath(const QString &path)
{
	if(!selectItemByPath(path)) {
		return false;
	}
	return addCurrentItemToWatchlist();
}

bool IIOExplorerInstrument_API::removeItemFromWatchlistByPath(const QString &path)
{
	QStringList pathList = path.split('/', Qt::SkipEmptyParts);
	QStandardItem *root = p->m_iioModel->getModel()->invisibleRootItem()->child(0);
	IIOStandardItem *iioRoot = dynamic_cast<IIOStandardItem *>(root);
	if(!iioRoot) {
		return false;
	}

	IIOStandardItem *item = p->findItemByPath(iioRoot, pathList);
	if(!item || !item->isWatched()) {
		return false;
	}

	item->setWatched(false);
	p->m_watchListView->removeFromWatchlist(item);

	// Update code generator
	QList<CodeGenerator::CodeGeneratorRecipe> recipes;
	for(auto witem : *p->m_watchListView->watchListEntries()) {
		recipes.append(CodeGenerator::convertToCodeGeneratorRecipe(witem->item(), p->m_uri));
	}
	p->m_generatedCodeBrowser->setPlainText(CodeGenerator::generateCode(recipes));

	return true;
}

QStringList IIOExplorerInstrument_API::getWatchlistPaths() { return p->m_watchListView->watchListEntries()->keys(); }

void IIOExplorerInstrument_API::clearWatchlist()
{
	QStringList paths = getWatchlistPaths();
	for(const QString &path : paths) {
		removeItemFromWatchlistByPath(path);
	}
}

void IIOExplorerInstrument_API::refreshWatchlist() { p->m_watchListView->refreshWatchlist(); }

bool IIOExplorerInstrument_API::writeWatchlistAttributeValue(const QString &path, const QString &value)
{
	QMap<QString, WatchListEntry *> *entries = p->m_watchListView->watchListEntries();
	if(!entries->contains(path)) {
		return false;
	}

	WatchListEntry *entry = entries->value(path);
	IIOStandardItem *item = entry->item();
	QList<IIOWidget *> widgets = item->getIIOWidgets();
	if(widgets.isEmpty()) {
		return false;
	}

	IIOWidget *widget = widgets.first();
	widget->getDataStrategy()->writeAsync(value);
	return true;
}

// ============================================
// ATTRIBUTE VALUES
// ============================================

QString IIOExplorerInstrument_API::readAttributeValue(const QString &path)
{
	QStringList pathList = path.split('/', Qt::SkipEmptyParts);
	QStandardItem *root = p->m_iioModel->getModel()->invisibleRootItem()->child(0);
	IIOStandardItem *iioRoot = dynamic_cast<IIOStandardItem *>(root);
	if(!iioRoot) {
		return QString();
	}

	IIOStandardItem *item = p->findItemByPath(iioRoot, pathList);
	if(!item) {
		return QString();
	}

	QList<IIOWidget *> widgets = item->getIIOWidgets();
	if(widgets.isEmpty()) {
		return QString();
	}

	IIOWidget *widget = widgets.first();
	// Use readAsync to trigger logging, then return the cached data
	widget->getDataStrategy()->readAsync();
	return widget->getDataStrategy()->data();
}

bool IIOExplorerInstrument_API::writeAttributeValue(const QString &path, const QString &value)
{
	QStringList pathList = path.split('/', Qt::SkipEmptyParts);
	QStandardItem *root = p->m_iioModel->getModel()->invisibleRootItem()->child(0);
	IIOStandardItem *iioRoot = dynamic_cast<IIOStandardItem *>(root);
	if(!iioRoot) {
		return false;
	}

	IIOStandardItem *item = p->findItemByPath(iioRoot, pathList);
	if(!item) {
		return false;
	}

	QList<IIOWidget *> widgets = item->getIIOWidgets();
	if(widgets.isEmpty()) {
		return false;
	}

	IIOWidget *widget = widgets.first();
	// Use writeAsync to trigger logging
	widget->getDataStrategy()->writeAsync(value);
	return true;
}

// must be called after write
void IIOExplorerInstrument_API::triggerRead()
{
	if(!p->m_currentlySelectedItem) {
		return;
	}
	p->triggerReadOnAllChildItems(p->m_currentlySelectedItem);
}

// ============================================
// CODE GENERATOR
// ============================================

QString IIOExplorerInstrument_API::getGeneratedCode() { return p->m_generatedCodeBrowser->toPlainText(); }

bool IIOExplorerInstrument_API::saveGeneratedCode(const QString &filePath)
{
	QFile file(filePath);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		return false;
	}
	file.write(p->m_generatedCodeBrowser->toPlainText().toUtf8());
	file.close();
	return true;
}

// ============================================
// LOG (using friend class access to m_textBrowser)
// ============================================

QString IIOExplorerInstrument_API::getLogContent() { return p->m_debugLogger->m_textBrowser->toPlainText(); }

void IIOExplorerInstrument_API::clearLog() { p->m_debugLogger->m_textBrowser->clear(); }

// ============================================
// SEARCH/FILTER
// ============================================

void IIOExplorerInstrument_API::setSearchText(const QString &text) { p->m_searchBar->getLineEdit()->setText(text); }

QString IIOExplorerInstrument_API::getSearchText() { return p->m_searchBar->getLineEdit()->text(); }

QString IIOExplorerInstrument_API::getSearchPlaceholderText()
{
	return p->m_searchBar->getLineEdit()->placeholderText();
}

int IIOExplorerInstrument_API::getVisibleItemCount()
{
	// Count visible items in the proxy model recursively
	std::function<int(const QModelIndex &)> countItems = [&](const QModelIndex &parent) -> int {
		int count = 0;
		int rows = p->m_proxyModel->rowCount(parent);
		for(int i = 0; i < rows; ++i) {
			QModelIndex index = p->m_proxyModel->index(i, 0, parent);
			count++;		    // Count this item
			count += countItems(index); // Count children recursively
		}
		return count;
	};

	return countItems(QModelIndex());
}

// ============================================
// DETAILS VIEW TAB (using friend class access to m_tabWidget)
// ============================================

void IIOExplorerInstrument_API::setDetailsViewTab(int index)
{
	if(index >= 0 && index < p->m_detailsView->m_tabWidget->count()) {
		p->m_detailsView->m_tabWidget->setCurrentIndex(index);
	}
}

int IIOExplorerInstrument_API::getDetailsViewTab() { return p->m_detailsView->m_tabWidget->currentIndex(); }

#include "moc_iioexplorerinstrument_api.cpp"
