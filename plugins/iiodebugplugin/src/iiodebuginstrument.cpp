#include <QVBoxLayout>
#include <QLabel>

#include <gui/stylehelper.h>

#include "iiodebuginstrument.h"
#include "iiostandarditem.h"
#include "debuggerloggingcategories.h"

using namespace scopy::iiodebugplugin;

IIODebugInstrument::IIODebugInstrument(struct iio_context *context, QString uri, QWidget *parent)
	: QWidget(parent)
	, m_context(context)
	, m_uri(uri)
	, m_currentlySelectedItem(nullptr)
{
	setObjectName("IIODebugInstrument - " + uri);
	setupUi();
	connectSignalsAndSlots();

	// api object for saving the state of widgets
	m_apiObject = new IIODebugInstrument_API(this);
	m_apiObject->setObjectName("IIODebugInstrument");
}

IIODebugInstrument::~IIODebugInstrument() {}

void IIODebugInstrument::saveSettings(QSettings &s)
{
	m_apiObject->save(s);
	m_watchListView->saveSettings(s);
}

void IIODebugInstrument::loadSettings(QSettings &s)
{
	m_apiObject->load(s);
	m_watchListView->loadSettings(s);
}

void IIODebugInstrument::setupUi()
{
	auto *search_bar_container = new QWidget(this);
	auto *bottom_container = new QWidget(this);
	auto *tree_view_container = new QWidget(bottom_container);
	auto *right_container = new QWidget(bottom_container);
	auto *details_container = new QWidget(right_container);
	auto *watch_list = new QWidget(right_container);
	m_VSplitter = new QSplitter(Qt::Vertical, this);
	m_HSplitter = new QSplitter(Qt::Horizontal, this);

	setLayout(new QVBoxLayout(this));
	bottom_container->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	bottom_container->setLayout(new QHBoxLayout(bottom_container));
	bottom_container->layout()->setContentsMargins(0, 0, 0, 0);

	right_container->setLayout(new QVBoxLayout(right_container));
	right_container->layout()->setContentsMargins(0, 0, 0, 0);
	details_container->setLayout(new QVBoxLayout(details_container));
	watch_list->setLayout(new QVBoxLayout(watch_list));
	tree_view_container->setLayout(new QVBoxLayout(tree_view_container));
	search_bar_container->setLayout(new QHBoxLayout(search_bar_container));
	search_bar_container->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	m_proxyModel = new IIOSortFilterProxyModel(this);
	m_treeView = new QTreeView(bottom_container);
	// m_saveContextSetup = new SaveContextSetup(m_treeView, bottom_container);
	// m_iioModel = new IIOModel(m_context, m_uri, m_treeView);

	// TODO: see what to do with context0 when multiple simultaneous connections are available
	m_iioModel = new IIOModel(m_context, "context0", m_treeView);
	m_searchBar = new SearchBar(m_iioModel->getEntries(), this);
	m_detailsView = new DetailsView(details_container);
	m_watchListView = new WatchListView(watch_list);

	watch_list->layout()->setContentsMargins(0, 0, 0, 0);
	watch_list->layout()->addWidget(m_watchListView);

	m_proxyModel->setSourceModel(m_iioModel->getModel());
	m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

	StyleHelper::BackgroundPage(details_container, "DetailsContainer");
	StyleHelper::BackgroundPage(watch_list, "WatchListContainer");
	StyleHelper::BackgroundPage(tree_view_container, "TreeViewContainer");
	StyleHelper::BackgroundPage(search_bar_container, "SearchBarContainer");
	StyleHelper::SplitterStyle(m_HSplitter, "HorizontalSplitter");
	StyleHelper::SplitterStyle(m_VSplitter, "VerticalSplitter");
	StyleHelper::TreeViewDebugger(m_treeView, "TreeView");

	m_treeView->setModel(m_proxyModel);

	// expand the root element, better visual experience
	m_treeView->expand(m_proxyModel->index(0, 0));

	details_container->layout()->addWidget(m_detailsView);
	tree_view_container->layout()->addWidget(m_treeView);
	search_bar_container->layout()->addWidget(m_searchBar);

	layout()->addWidget(search_bar_container);
	layout()->addWidget(bottom_container);
	m_HSplitter->addWidget(tree_view_container);
	m_HSplitter->addWidget(right_container);
	bottom_container->layout()->addWidget(m_HSplitter);
	m_VSplitter->addWidget(details_container);
	m_VSplitter->addWidget(watch_list);
	right_container->layout()->addWidget(m_VSplitter);
}

void IIODebugInstrument::connectSignalsAndSlots()
{
	QObject::connect(m_searchBar->getLineEdit(), &QLineEdit::textChanged, this, [this](QString text) {
		if(text.isEmpty()) {
			auto sourceModel = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
			m_proxyModel->setFilterRegExp(QRegExp("", Qt::CaseInsensitive, QRegExp::FixedString));
			m_proxyModel->invalidate(); // Trigger re-filtering
			collapseAllItems(sourceModel->invisibleRootItem());
			m_treeView->expand(m_proxyModel->index(0, 0));
		} else {
			filterAndExpand(text);
		}
	});

	QObject::connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
			 &IIODebugInstrument::applySelection);

	QObject::connect(m_watchListView, &WatchListView::selectedItem, this, [this](IIOStandardItem *item) {
		qDebug(CAT_DEBUGGERIIOMODEL) << "Expanding item" << item->path();
		m_currentlySelectedItem = item;
		auto sourceModel = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
		recursiveExpandItem(sourceModel->invisibleRootItem(), item);
		m_detailsView->setIIOStandardItem(item);
	});

	QObject::connect(m_detailsView->readBtn(), &QPushButton::clicked, this, [this]() {
		qInfo(CAT_DEBUGGERIIOMODEL) << "Read button pressed.";
		triggerReadOnAllChildItems(m_currentlySelectedItem);
		m_detailsView->refreshIIOView();
	});

	QObject::connect(m_detailsView->addToWatchlistBtn(), &QPushButton::clicked, this, [this]() {
		if(m_currentlySelectedItem == nullptr) {
			qInfo(CAT_DEBUGGERIIOMODEL) << "No item selected, doing nothing.";
			return;
		}

		if(m_currentlySelectedItem->isWatched()) {
			m_currentlySelectedItem->setWatched(false);
			m_watchListView->removeFromWatchlist(m_currentlySelectedItem);
			m_detailsView->setAddToWatchlistState(true);
		} else {
			m_currentlySelectedItem->setWatched(true);
			m_watchListView->addToWatchlist(m_currentlySelectedItem);
			m_detailsView->setAddToWatchlistState(false);
		}
	});
}

IIOStandardItem *IIODebugInstrument::findItemRecursive(QStandardItem *currentItem, QStandardItem *targetItem)
{
	if(currentItem == targetItem) {
		return dynamic_cast<IIOStandardItem *>(currentItem);
	}

	// Check children recursively
	for(int i = 0; i < currentItem->rowCount(); ++i) {
		IIOStandardItem *result = findItemRecursive(currentItem->child(i), targetItem);
		if(result) {
			return result;
		}
	}

	return nullptr;
}

void IIODebugInstrument::recursiveExpandItems(QStandardItem *item, const QString &text)
{
	for(int row = 0; row < item->rowCount(); ++row) {
		QStandardItem *childItem = item->child(row);

		if(childItem == nullptr) {
			qWarning(CAT_DEBUGGERIIOMODEL) << "Error, invalid row. Skipping item.";
			continue;
		}

		// Check if the item's data contains the filter string
		if(childItem->text().contains(text, Qt::CaseInsensitive)) {
			QModelIndex proxyIndex = m_proxyModel->mapFromSource(childItem->index());

			// Expand all parents from the root to the current element
			QModelIndex parent = proxyIndex.parent();
			while(parent.isValid()) {
				m_treeView->expand(parent);
				parent = parent.parent();
			}
		}

		// Recursively process children
		recursiveExpandItems(childItem, text);
	}
}

void IIODebugInstrument::recursiveExpandItem(QStandardItem *item, QStandardItem *searchItem)
{
	for(int row = 0; row < item->rowCount(); ++row) {
		QStandardItem *childItem = item->child(row);

		if(childItem == searchItem) {
			QModelIndex index = m_proxyModel->mapFromSource(childItem->index());
			QModelIndex parent = index.parent();

			// Recursively expand all parents once the item was found
			while(parent.isValid()) {
				m_treeView->expand(parent);
				parent = parent.parent();
			}

			// Highlight the selected item
			QItemSelectionModel *selectionModel = m_treeView->selectionModel();
			selectionModel->select(index, QItemSelectionModel::ClearAndSelect);
			return;
		}

		recursiveExpandItem(childItem, searchItem);
	}
}

void IIODebugInstrument::collapseAllItems(QStandardItem *item)
{
	for(int row = 0; row < item->rowCount(); ++row) {
		QStandardItem *childItem = item->child(row);
		QModelIndex proxyIndex = m_proxyModel->mapFromSource(childItem->index());
		collapseAllItems(childItem);
		m_treeView->collapse(proxyIndex);
	}
}

void IIODebugInstrument::triggerReadOnAllChildItems(QStandardItem *item)
{
	auto IIOitem = dynamic_cast<IIOStandardItem *>(item);
	if(!IIOitem) {
		qWarning(CAT_DEBUGGERIIOMODEL) << "Dynamic cast failed and it shouldn't.";
		return;
	}
	IIOStandardItem::Type type = IIOitem->type();

	// if it is a leaf node, trigger read for all (most probably only one) IIOWidgets it contains
	if(type == IIOStandardItem::ContextAttribute || type == IIOStandardItem::DeviceAttribute ||
	   type == IIOStandardItem::ChannelAttribute) {
		QList<IIOWidget *> iioWidgets = IIOitem->getIIOWidgets();
		for(int i = 0; i < iioWidgets.size(); ++i) {
			qDebug(CAT_DEBUGGERIIOMODEL) << "Reading " << IIOitem->path();
			iioWidgets.at(i)->getDataStrategy()->requestData();
		}
	} else {
		// not a leaf node, continue recursion
		for(int i = 0; i < item->rowCount(); ++i) {
			QStandardItem *child = item->child(i);
			triggerReadOnAllChildItems(child);
		}
	}
}

void IIODebugInstrument::applySelection(const QItemSelection &selected, const QItemSelection &deselected)
{
	Q_UNUSED(deselected)

	if(selected.indexes().isEmpty()) {
		qWarning(CAT_DEBUGGERIIOMODEL) << "Selected index is cannot be found.";
		return;
	}

	QModelIndex proxyIndex = selected.indexes().first();

	// Map the proxy index to the source index
	QModelIndex sourceIndex = m_proxyModel->mapToSource(proxyIndex);

	// Get the source model item from the source index
	QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());
	QStandardItem *modelItem = sourceModel->itemFromIndex(sourceIndex);

	// Find the root item from the source model
	QStandardItem *rootItem = sourceModel->invisibleRootItem();

	// Use a recursive function to find the corresponding item in the source model
	IIOStandardItem *iioItem = findItemRecursive(rootItem, modelItem);
	m_currentlySelectedItem = iioItem;

	if(iioItem) {
		m_detailsView->setAddToWatchlistState(!iioItem->isWatched());
		m_detailsView->setIIOStandardItem(iioItem);
		m_watchListView->currentTreeSelectionChanged(iioItem);
	}
}

void IIODebugInstrument::filterAndExpand(const QString &text)
{
	m_proxyModel->setFilterRegExp(QRegExp(text, Qt::CaseInsensitive, QRegExp::FixedString));
	m_proxyModel->invalidate(); // Trigger re-filtering

	if(text.isEmpty()) {
		qDebug(CAT_DEBUGGERIIOMODEL) << "Text is empty, will not recursively expand items.";
		return;
	}

	QStandardItemModel *sourceModel = qobject_cast<QStandardItemModel *>(m_proxyModel->sourceModel());

	if(!sourceModel) {
		qWarning(CAT_DEBUGGERIIOMODEL) << "Failed to obtain source model.";
		return;
	}

	// Iterate through the items in the source model
	recursiveExpandItems(sourceModel->invisibleRootItem(), text);
}

// --------------------------------------------------------

QList<int> IIODebugInstrument_API::vSplitter() const { return p->m_VSplitter->sizes(); }

void IIODebugInstrument_API::setVSplitter(const QList<int> &newSplitter) { p->m_VSplitter->setSizes(newSplitter); }

QList<int> IIODebugInstrument_API::hSplitter() const { return p->m_HSplitter->sizes(); }

void IIODebugInstrument_API::setHSplitter(const QList<int> &newSplitter) { p->m_HSplitter->setSizes(newSplitter); }
