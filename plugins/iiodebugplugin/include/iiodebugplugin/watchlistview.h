#ifndef SCOPY_WATCHLISTVIEW_H
#define SCOPY_WATCHLISTVIEW_H

#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <pluginbase/apiobject.h>
#include "watchlistentry.h"
#include "iiostandarditem.h"

namespace scopy::iiodebugplugin {
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

Q_SIGNALS:
	void selectedItem(IIOStandardItem *item);

protected:
	void resizeEvent(QResizeEvent *event) override;

private:
	QList<int> m_offsets = {0, 0, 0, 0}; // TODO: prettify

	ApiObject *m_apiObject;
	QMap<QString, WatchListEntry *> m_entryObjects;
};

class WatchListView_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(QList<int> offsets READ offsets WRITE setOffsets FINAL)
public:
	explicit WatchListView_API(WatchListView *p)
		: ApiObject(p)
		, p(p)
	{}
	~WatchListView_API(){};

	QList<int> tableHeader() const;
	void setTableHeader(const QList<int> &newTableHeader);

	QList<int> offsets() const;
	void setOffsets(const QList<int> &newOffsets);

private:
	WatchListView *p;
};
} // namespace scopy::iiodebugplugin

#endif // SCOPY_WATCHLISTVIEW_H
