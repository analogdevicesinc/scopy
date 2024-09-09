#ifndef IIOEXPLORERINSTRUMENT_H
#define IIOEXPLORERINSTRUMENT_H

#include "scopy-debugger_export.h"
#include "iiomodel.h"
#include "detailsview.h"
#include "searchbar.h"
#include "iiosortfilterproxymodel.h"
#include "watchlistview.h"
#include "pluginbase/apiobject.h"
#include "savecontextsetup.h"
#include "iiodebuglogger.h"

#include <iio.h>
#include <QWidget>
#include <QTreeView>
#include <QSplitter>
#include <QTabWidget>

namespace scopy::debugger {
class SCOPY_DEBUGGER_EXPORT IIOExplorerInstrument : public QWidget
{
	Q_OBJECT
	friend class IIOExplorerInstrument_API;

public:
	IIOExplorerInstrument(struct iio_context *context, QString uri, QWidget *parent = nullptr);
	~IIOExplorerInstrument();

	void saveSettings(QSettings &);
	void loadSettings(QSettings &);

private Q_SLOTS:
	void applySelection(const QItemSelection &selected, const QItemSelection &deselected);
	void filterAndExpand(const QString &text);
	void selectItem(IIOStandardItem *item);

private:
	void setupUi();
	void connectSignalsAndSlots();

	// Recursive function to find an item in the source model
	IIOStandardItem *findItemRecursive(QStandardItem *currentItem, QStandardItem *targetItem);

	// Recursively expand all items (and their parents) with the title matching the text param
	void recursiveExpandItems(QStandardItem *item, const QString &text);

	// Expand the searchItem and all its parents
	void recursiveExpandItem(QStandardItem *item, QStandardItem *searchItem);

	// Collapse all items starting from the item parameter
	void collapseAllItems(QStandardItem *item);

	// Triggers a read operation on the item specified as parameter and all children of that item recursively
	void triggerReadOnAllChildItems(QStandardItem *item);

	// Finds the item containing the path variable
	IIOStandardItem *findItemByPath(IIOStandardItem *currentItem, const QStringList &path, int depth = 0);

	// Stacked widget
	QTabWidget *m_tabWidget;
	QWidget *m_mainWidget;
	IIODebugLogger *m_debugLogger;

	struct iio_context *m_context;
	QString m_uri;
	QTreeView *m_treeView;
	IIOModel *m_iioModel;
	DetailsView *m_detailsView;
	SearchBar *m_searchBar;
	IIOSortFilterProxyModel *m_proxyModel;
	WatchListView *m_watchListView;
	ApiObject *m_apiObject;
	IIOStandardItem *m_currentlySelectedItem;
	SaveContextSetup *m_saveContextSetup;

	// GUI elements
	QSplitter *m_VSplitter;
	QSplitter *m_HSplitter;
};

class SCOPY_DEBUGGER_EXPORT IIOExplorerInstrument_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(QList<int> vSplitter READ vSplitter WRITE setVSplitter FINAL)
	Q_PROPERTY(QList<int> hSplitter READ hSplitter WRITE setHSplitter FINAL)

public:
	explicit IIOExplorerInstrument_API(IIOExplorerInstrument *p)
		: ApiObject(p)
		, p(p)
	{}
	~IIOExplorerInstrument_API(){};

	QList<int> vSplitter() const;
	void setVSplitter(const QList<int> &newSplitter);

	QList<int> hSplitter() const;
	void setHSplitter(const QList<int> &newSplitter);

private:
	IIOExplorerInstrument *p;
};
} // namespace scopy::debugger
#endif // IIOEXPLORERINSTRUMENT_H
