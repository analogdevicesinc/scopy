#ifndef IIOWIDGETSELECTOR_H
#define IIOWIDGETSELECTOR_H

#include <QWidget>
#include <QFrame>
#include <QStandardItemModel>
#include <QItemSelection>
#include <QTreeView>
#include <iioutil/iioitem.h>

namespace scopy {
class IIOWidgetSelector : public QFrame
{
	Q_OBJECT
public:
	IIOWidgetSelector(IIOItem *root, QWidget *parent = nullptr);
	~IIOWidgetSelector();
	QTreeView *getTree() const;
	QStandardItemModel *getModel() const;

Q_SIGNALS:
	void itemSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
	QTreeView *m_treeView;
	QStandardItemModel *m_model;
};
} // namespace scopy

#endif // IIOWIDGETSELECTOR_H
