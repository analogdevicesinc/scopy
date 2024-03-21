#ifndef IIODEBUGINSTRUMENT_H
#define IIODEBUGINSTRUMENT_H

#include "scopy-iiodebugplugin_export.h"
#include "iiomodel.h"
#include "detailsview.h"
#include "searchbar.h"
#include "iiosortfilterproxymodel.h"
#include "watchlistview.h"
#include "pluginbase/apiobject.h"
#include "savecontextsetup.h"

#include <iio.h>
#include <QWidget>
#include <QTreeView>
#include <QSplitter>

namespace scopy::iiodebugplugin {
class SCOPY_IIODEBUGPLUGIN_EXPORT IIODebugInstrument : public QWidget
{
	Q_OBJECT
	friend class IIODebugInstrument_API;

public:
	IIODebugInstrument(struct iio_context *context, QString uri, QWidget *parent = nullptr);
	~IIODebugInstrument();

	void saveSettings(QSettings &);
	void loadSettings(QSettings &);

private Q_SLOTS:
	void applySelection(const QItemSelection &selected, const QItemSelection &deselected);
	void filterAndExpand(const QString &text);

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

	struct iio_context *m_context;
	QString m_uri;
	QTreeView *m_treeView;
	IIOModel *m_iioModel;
	DetailsView *m_detailsView;
	SearchBar *m_searchBar;
	IIOSortFilterProxyModel *m_proxyModel;
	WatchListView *m_watchListView;
	ApiObject *m_apiObject;
	QPushButton *m_readBtn;
	QPushButton *m_addToWatchlistBtn;
	IIOStandardItem *m_currentlySelectedItem;
	SaveContextSetup *m_saveContextSetup;

	// GUI elements
	QSplitter *m_VSplitter;
	QSplitter *m_HSplitter;
};

class SCOPY_IIODEBUGPLUGIN_EXPORT IIODebugInstrument_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(QList<int> vSplitter READ vSplitter WRITE setVSplitter FINAL)
	Q_PROPERTY(QList<int> hSplitter READ hSplitter WRITE setHSplitter FINAL)

public:
	explicit IIODebugInstrument_API(IIODebugInstrument *p)
		: ApiObject(p)
		, p(p)
	{}
	~IIODebugInstrument_API(){};

	QList<int> vSplitter() const;
	void setVSplitter(const QList<int> &newSplitter);

	QList<int> hSplitter() const;
	void setHSplitter(const QList<int> &newSplitter);

private:
	IIODebugInstrument *p;
};
} // namespace scopy::iiodebugplugin
#endif // IIODEBUGINSTRUMENT_H
