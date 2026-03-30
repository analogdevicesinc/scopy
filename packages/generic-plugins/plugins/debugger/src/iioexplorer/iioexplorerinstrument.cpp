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

#include <QVBoxLayout>
#include <QLabel>
#include <QScrollBar>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QFileDialog>
#include <style.h>
#include <gui/stylehelper.h>
#include <pluginbase/apiobject.h>
#include <pluginbase/scopyjs.h>
#include "iioexplorerinstrument.h"
#include "iioexplorerinstrument_api.h"
#include "hoverwidget.h"
#include "iiostandarditem.h"
#include "debuggerloggingcategories.h"
#include "style_properties.h"

using namespace scopy::debugger;

IIOExplorerInstrument::IIOExplorerInstrument(struct iio_context *context, QString uri, QWidget *parent)
	: QWidget(parent)
	, m_context(context)
	, m_uri(uri)
	, m_currentlySelectedItem(nullptr)
	, m_currentDetailsPage(nullptr)
{
	setObjectName("IIOExplorerInstrument - " + uri);
	setupUi();
	connectSignalsAndSlots();

	// api object for saving the state of widgets
	m_apiObject = new IIOExplorerInstrument_API(this);
	m_apiObject->setObjectName("iioExplorer");
	ScopyJS::GetInstance()->registerApi(m_apiObject);
}

IIOExplorerInstrument::~IIOExplorerInstrument() { ScopyJS::GetInstance()->unregisterApi(m_apiObject); }

void IIOExplorerInstrument::saveSettings(QSettings &s)
{
	m_apiObject->save(s);
	m_watchListView->saveSettings(s);
}

void IIOExplorerInstrument::loadSettings(QSettings &s)
{
	m_apiObject->load(s);
	m_watchListView->loadSettings(s);
}

void IIOExplorerInstrument::setupUi()
{
	setMinimumSize(720, 480);
	m_tabWidget = new QTabWidget(this);

	m_mainWidget = new QWidget(m_tabWidget);
	m_mainWidget->setObjectName("IIODebuggerMainWidget");

	m_debugLogger = new IIODebugLogger(m_tabWidget);
	m_debugLogger->setObjectName("IIODebuggerLogWidget");

	m_codeGenerator = new QWidget(m_tabWidget);
	m_codeGenerator->setObjectName("IIODebuggerCodeGeneratorWidget");
	setupCodeGeneratorWindow();

	m_tabWidget->addTab(m_mainWidget, "IIO Attributes");
	m_tabWidget->addTab(m_debugLogger, "Log");
	m_tabWidget->addTab(m_codeGenerator, "Code Generator");

	m_tabWidget->setCurrentIndex(0);
	m_tabWidget->setTabPosition(QTabWidget::South);

	QWidget *bottom_container = new QWidget(m_mainWidget);
	m_VSplitter = new QSplitter(Qt::Vertical, this);
	m_HSplitter = new QSplitter(Qt::Horizontal, bottom_container);
	QWidget *tree_view_container = new QWidget(m_HSplitter);
	QWidget *right_container = new QWidget(m_HSplitter);
	QWidget *details_container = new QWidget(right_container);
	QWidget *watch_list = new QWidget(right_container);

	Style::setBackgroundColor(details_container, json::theme::background_subtle);
	Style::setBackgroundColor(tree_view_container, json::theme::background_subtle);

	m_mainWidget->setLayout(new QVBoxLayout(m_mainWidget));
	m_mainWidget->layout()->setContentsMargins(0, 0, 0, 0);

	bottom_container->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	bottom_container->setLayout(new QHBoxLayout(bottom_container));
	bottom_container->layout()->setContentsMargins(0, 0, 0, 0);

	right_container->setLayout(new QVBoxLayout(right_container));
	right_container->layout()->setContentsMargins(0, 0, 0, 0);
	details_container->setLayout(new QVBoxLayout(details_container));
	details_container->layout()->setContentsMargins(2, 9, 9, 2);
	watch_list->setLayout(new QVBoxLayout(watch_list));
	watch_list->layout()->setContentsMargins(2, 2, 9, 9);
	tree_view_container->setLayout(new QVBoxLayout(tree_view_container));
	tree_view_container->layout()->setContentsMargins(9, 9, 2, 9);

	m_proxyModel = new IIOSortFilterProxyModel(this);
	m_treeView = new QTreeView(tree_view_container);
	m_treeView->setHeaderHidden(true);

	m_iioModel = new IIOModel(m_context, "context0", m_treeView);
	m_searchBar = new SearchBar(m_iioModel->getEntries(), this);
	m_mapStack = new LazyStackedWidget(this, details_container);
	m_watchListView = new WatchListView(watch_list);

	watch_list->layout()->addWidget(m_watchListView);

	m_proxyModel->setSourceModel(m_iioModel->getModel());
	m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

	Style::setBackgroundColor(m_mainWidget, json::theme::background_subtle);
	Style::setBackgroundColor(m_debugLogger, json::theme::background_subtle);
	Style::setBackgroundColor(m_codeGenerator, json::theme::background_subtle);
	Style::setStyle(m_treeView, style::properties::debugger::treeView);

	m_treeView->setModel(m_proxyModel);

	// Expand the root element and build its page
	m_treeView->expand(m_proxyModel->index(0, 0));
	m_currentlySelectedItem =
		dynamic_cast<IIOStandardItem *>(m_iioModel->getModel()->invisibleRootItem()->child(0));
	m_mapStack->show(m_currentlySelectedItem->path());

	details_container->layout()->addWidget(m_mapStack);
	tree_view_container->layout()->addWidget(m_searchBar);
	tree_view_container->layout()->addWidget(m_treeView);

	m_mainWidget->layout()->addWidget(bottom_container);
	m_HSplitter->addWidget(tree_view_container);
	m_HSplitter->addWidget(right_container);
	bottom_container->layout()->addWidget(m_HSplitter);
	m_VSplitter->addWidget(details_container);
	m_VSplitter->addWidget(watch_list);
	right_container->layout()->addWidget(m_VSplitter);

	setLayout(new QVBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->addWidget(m_tabWidget);
}

void IIOExplorerInstrument::connectSignalsAndSlots()
{
	QObject::connect(m_searchBar->getLineEdit(), &QLineEdit::textChanged, this, [this](QString text) {
		if(text.isEmpty()) {
			auto sourceModel = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
			m_proxyModel->setFilterRegExp(QRegExp("", Qt::CaseInsensitive, QRegExp::FixedString));
			m_proxyModel->invalidate();
			collapseAllItems(sourceModel->invisibleRootItem());
			m_treeView->expand(m_proxyModel->index(0, 0));
		} else {
			filterAndExpand(text);
		}
	});

	QObject::connect(m_treeView, &QTreeView::expanded, this, [this](const QModelIndex &proxyIndex) {
		QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);
		QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
		IIOStandardItem *item = dynamic_cast<IIOStandardItem *>(sourceModel->itemFromIndex(sourceIndex));
		if(item) {
			m_iioModel->populateChildren(item);
		}
	});

	QObject::connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
			 &IIOExplorerInstrument::applySelection);

	QObject::connect(m_watchListView, &WatchListView::selectedItem, this, &IIOExplorerInstrument::selectItem);

	QObject::connect(m_watchListView, &WatchListView::removeItem, this, [this](IIOStandardItem *item) {
		item->setWatched(false);
		if(m_currentDetailsPage) {
			m_currentDetailsPage->setAddToWatchlistState(true);
		}

		QList<CodeGenerator::CodeGeneratorRecipe> recipes;
		for(auto witem : *m_watchListView->watchListEntries()) {
			recipes.append(CodeGenerator::convertToCodeGeneratorRecipe(witem->item(), m_uri));
		}
		m_generatedCodeBrowser->setPlainText(CodeGenerator::generateCode(recipes));
	});

	QObject::connect(
		m_iioModel, &IIOModel::emitLog, this,
		[this](QDateTime timestamp, bool isRead, QString path, QString oldValue, QString newValue,
		       int returnCode) {
			QString logMessage =
				QString("[%1] %2 (%3): %4: %5%6")
					.arg(timestamp.toString("hh:mm:ss"), isRead ? "R" : "W",
					     returnCode < 0 ? "FAILURE " + QString::number(returnCode) : "SUCCESS",
					     path, oldValue.isEmpty() || isRead ? "" : oldValue + " -> ", newValue);
			m_debugLogger->appendLog(logMessage);
			if(m_currentDetailsPage) {
				m_currentDetailsPage->refreshIIOView();
			}
		});
}

QWidget *IIOExplorerInstrument::createWidget(const QString &key)
{
	QStringList pathList = key.split('/', Qt::SkipEmptyParts);
	QStandardItem *root = m_iioModel->getModel()->invisibleRootItem()->child(0);
	IIOStandardItem *iioRoot = dynamic_cast<IIOStandardItem *>(root);
	if(!iioRoot) {
		qWarning(CAT_IIODEBUGGER) << "Cannot find the model root.";
		return nullptr;
	}

	IIOStandardItem *item = findItemByPath(iioRoot, pathList);
	if(!item) {
		qWarning(CAT_IIODEBUGGER) << "Could not find item for key:" << key;
		return nullptr;
	}

	m_iioModel->populateChildren(item);

	auto *page = new DetailsPage(item, m_uri, m_mapStack);

	connect(page, &DetailsPage::pathSelected, this, [this](QString path) {
		QStringList pathList = path.split('/', Qt::SkipEmptyParts);
		QStandardItem *root = m_iioModel->getModel()->invisibleRootItem()->child(0);
		IIOStandardItem *iioRoot = dynamic_cast<IIOStandardItem *>(root);
		if(!iioRoot) {
			qWarning(CAT_IIODEBUGGER) << "Cannot find the model root.";
			return;
		}
		IIOStandardItem *foundItem = findItemByPath(iioRoot, pathList);
		if(!foundItem) {
			qWarning(CAT_IIODEBUGGER) << "Could not find the item with path:" << path;
			return;
		}
		selectItem(foundItem);
	});

	connect(page->readBtn(), &QPushButton::clicked, this, &IIOExplorerInstrument::onReadAllClicked);
	connect(page->addToWatchlistBtn(), &QPushButton::clicked, this,
		&IIOExplorerInstrument::onWatchlistToggleClicked);

	return page;
}

void IIOExplorerInstrument::onShow(const QString &key, QWidget *widget)
{
	Q_UNUSED(key)
	m_currentDetailsPage = qobject_cast<DetailsPage *>(widget);
	if(m_currentDetailsPage && m_currentlySelectedItem) {
		m_currentDetailsPage->setAddToWatchlistState(!m_currentlySelectedItem->isWatched());
	}
}

void IIOExplorerInstrument::onRemove(const QString &key, QWidget *widget)
{
	Q_UNUSED(key)
	if(m_currentDetailsPage == widget) {
		m_currentDetailsPage = nullptr;
	}
}

void IIOExplorerInstrument::onReadAllClicked()
{
	if(!m_currentDetailsPage || !m_currentlySelectedItem) {
		return;
	}
	qDebug(CAT_DEBUGGERIIOMODEL) << "Read button pressed.";
	m_currentDetailsPage->readBtn()->startAnimation();

	QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
	QObject::connect(
		watcher, &QFutureWatcher<void>::finished, this,
		[this, watcher]() {
			if(m_currentDetailsPage) {
				m_currentDetailsPage->refreshIIOView();
			}
			m_watchListView->refreshWatchlist();
			if(m_currentDetailsPage) {
				m_currentDetailsPage->readBtn()->stopAnimation();
			}
			watcher->deleteLater();
		},
		Qt::QueuedConnection);

	QFuture<void> future = QtConcurrent::run([this]() { triggerReadOnAllChildItems(m_currentlySelectedItem); });
	watcher->setFuture(future);
}

void IIOExplorerInstrument::onWatchlistToggleClicked()
{
	if(!m_currentlySelectedItem) {
		qInfo(CAT_DEBUGGERIIOMODEL) << "No item selected, doing nothing.";
		return;
	}

	if(m_currentlySelectedItem->isWatched()) {
		m_currentlySelectedItem->setWatched(false);
		m_watchListView->removeFromWatchlist(m_currentlySelectedItem);
		if(m_currentDetailsPage) {
			m_currentDetailsPage->setAddToWatchlistState(true);
		}
	} else {
		m_currentlySelectedItem->setWatched(true);
		m_watchListView->addToWatchlist(m_currentlySelectedItem);
		if(m_currentDetailsPage) {
			m_currentDetailsPage->setAddToWatchlistState(false);
		}
	}

	QList<CodeGenerator::CodeGeneratorRecipe> recipes;
	for(auto item : *m_watchListView->watchListEntries()) {
		recipes.append(CodeGenerator::convertToCodeGeneratorRecipe(item->item(), m_uri));
	}
	m_generatedCodeBrowser->setPlainText(CodeGenerator::generateCode(recipes));
}

IIOStandardItem *IIOExplorerInstrument::findItemRecursive(QStandardItem *currentItem, QStandardItem *targetItem)
{
	if(currentItem == targetItem) {
		return dynamic_cast<IIOStandardItem *>(currentItem);
	}

	for(int i = 0; i < currentItem->rowCount(); ++i) {
		IIOStandardItem *result = findItemRecursive(currentItem->child(i), targetItem);
		if(result) {
			return result;
		}
	}

	return nullptr;
}

void IIOExplorerInstrument::recursiveExpandItems(QStandardItem *item, const QString &text)
{
	for(int row = 0; row < item->rowCount(); ++row) {
		QStandardItem *childItem = item->child(row);

		if(childItem == nullptr) {
			qWarning(CAT_DEBUGGERIIOMODEL) << "Error, invalid row. Skipping item.";
			continue;
		}

		if(childItem->text().contains(text, Qt::CaseInsensitive)) {
			QModelIndex proxyIndex = m_proxyModel->mapFromSource(childItem->index());

			QModelIndex parent = proxyIndex.parent();
			while(parent.isValid()) {
				m_treeView->expand(parent);
				parent = parent.parent();
			}
		}

		recursiveExpandItems(childItem, text);
	}
}

void IIOExplorerInstrument::recursiveExpandItem(QStandardItem *item, QStandardItem *searchItem)
{
	for(int row = 0; row < item->rowCount(); ++row) {
		QStandardItem *childItem = item->child(row);

		if(childItem == searchItem) {
			QModelIndex index = m_proxyModel->mapFromSource(childItem->index());
			QModelIndex parent = index.parent();

			while(parent.isValid()) {
				m_treeView->expand(parent);
				parent = parent.parent();
			}

			QItemSelectionModel *selectionModel = m_treeView->selectionModel();
			selectionModel->select(index, QItemSelectionModel::ClearAndSelect);
			return;
		}

		recursiveExpandItem(childItem, searchItem);
	}
}

void IIOExplorerInstrument::collapseAllItems(QStandardItem *item)
{
	for(int row = 0; row < item->rowCount(); ++row) {
		QStandardItem *childItem = item->child(row);
		QModelIndex proxyIndex = m_proxyModel->mapFromSource(childItem->index());
		collapseAllItems(childItem);
		m_treeView->collapse(proxyIndex);
	}
}

void IIOExplorerInstrument::triggerReadOnAllChildItems(QStandardItem *item)
{
	auto IIOitem = dynamic_cast<IIOStandardItem *>(item);
	if(!IIOitem) {
		qWarning(CAT_DEBUGGERIIOMODEL) << "Dynamic cast failed and it shouldn't.";
		return;
	}
	IIOStandardItem::Type type = IIOitem->type();

	if(type == IIOStandardItem::ContextAttribute || type == IIOStandardItem::DeviceAttribute ||
	   type == IIOStandardItem::ChannelAttribute) {
		QList<IIOWidget *> iioWidgets = IIOitem->getIIOWidgets();
		for(int i = 0; i < iioWidgets.size(); ++i) {
			qDebug(CAT_DEBUGGERIIOMODEL) << "Reading " << IIOitem->path();
			iioWidgets.at(i)->getDataStrategy()->readAsync();
		}
	} else {
		// Ensure children are populated before recursing (handles API-triggered reads)
		m_iioModel->populateChildren(IIOitem);
		for(int i = 0; i < item->rowCount(); ++i) {
			QStandardItem *child = item->child(i);
			triggerReadOnAllChildItems(child);
		}
	}
}

IIOStandardItem *IIOExplorerInstrument::findItemByPath(IIOStandardItem *currentItem, const QStringList &path, int depth)
{
	if(!currentItem || depth >= path.size()) {
		return nullptr;
	}

	if(currentItem->name() == path[depth] || currentItem->id() == path[depth]) {
		if(depth == path.size() - 1) {
			return currentItem;
		}

		// Ensure children are populated before recursing so attribute items exist
		m_iioModel->populateChildren(currentItem);

		for(int i = 0; i < currentItem->rowCount(); ++i) {
			IIOStandardItem *child = dynamic_cast<IIOStandardItem *>(currentItem->child(i));
			IIOStandardItem *result = findItemByPath(child, path, depth + 1);
			if(result) {
				return result;
			}
		}
	}

	return nullptr;
}

void IIOExplorerInstrument::applySelection(const QItemSelection &selected, const QItemSelection &deselected)
{
	Q_UNUSED(deselected)

	if(selected.indexes().isEmpty()) {
		qWarning(CAT_DEBUGGERIIOMODEL) << "Selected index is cannot be found.";
		return;
	}

	QModelIndex proxyIndex = selected.indexes().first();
	QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);

	QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
	QStandardItem *modelItem = sourceModel->itemFromIndex(sourceIndex);
	QStandardItem *rootItem = sourceModel->invisibleRootItem();

	IIOStandardItem *iioItem = findItemRecursive(rootItem, modelItem);
	m_currentlySelectedItem = iioItem;

	if(iioItem) {
		m_mapStack->show(iioItem->path());
		m_watchListView->currentTreeSelectionChanged(iioItem);
	}
}

void IIOExplorerInstrument::filterAndExpand(const QString &text)
{
	m_proxyModel->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive, QRegExp::FixedString));
	m_proxyModel->invalidate();

	if(text.isEmpty()) {
		qDebug(CAT_DEBUGGERIIOMODEL) << "Text is empty, will not recursively expand items.";
		return;
	}

	QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
	if(!sourceModel) {
		qWarning(CAT_DEBUGGERIIOMODEL) << "Failed to obtain source model.";
		return;
	}

	recursiveExpandItems(sourceModel->invisibleRootItem(), text);
}

void IIOExplorerInstrument::selectItem(IIOStandardItem *item)
{
	qDebug(CAT_DEBUGGERIIOMODEL) << "Expanding item" << item->path();
	m_currentlySelectedItem = item;
	auto sourceModel = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
	recursiveExpandItem(sourceModel->invisibleRootItem(), item);
	m_mapStack->show(item->path());
}

void IIOExplorerInstrument::setupCodeGeneratorWindow()
{
	m_codeGenerator->setLayout(new QVBoxLayout(m_codeGenerator));
	m_codeGenerator->layout()->setContentsMargins(0, 0, 0, 0);

	m_generatedCodeBrowser = new QTextBrowser(m_codeGenerator);
	m_generatedCodeBrowser->setObjectName("CodeGeneratorTextBrowser");
	m_codeGenerator->layout()->addWidget(m_generatedCodeBrowser);
	Style::setStyle(m_generatedCodeBrowser, style::properties::widget::textBrowser);
	m_generatedCodeBrowser->setPlaceholderText("// Add items to the watchlist to generate code.");

	QPushButton *saveButton = new QPushButton("Save Code");
	Style::setStyle(saveButton, style::properties::button::basicButton, true, true);
	saveButton->setObjectName("SaveCodeButton");
	connect(saveButton, &QPushButton::clicked, this, [this]() {
		QString fileName = QFileDialog::getSaveFileName(this, "Save Code", "", "C++ Files (*.cpp *.h)");
		if(!fileName.isEmpty()) {
			QFile file(fileName);
			if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
				file.write(m_generatedCodeBrowser->toPlainText().toUtf8());
				file.close();
			}
		}
	});

	HoverWidget *hoverWidget = new HoverWidget(saveButton, m_generatedCodeBrowser, m_generatedCodeBrowser);
	hoverWidget->setObjectName("SaveCodeHoverWidget");
	hoverWidget->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	hoverWidget->setContentPos(HoverPosition::HP_BOTTOMLEFT);
	hoverWidget->setAnchorOffset(QPoint(-10, 10));
	hoverWidget->setVisible(true);
	hoverWidget->raise();
}

#include "moc_iioexplorerinstrument.cpp"
