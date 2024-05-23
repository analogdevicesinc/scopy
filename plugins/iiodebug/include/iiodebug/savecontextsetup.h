#ifndef SAVECONTEXTSETUP_H
#define SAVECONTEXTSETUP_H

#include <QObject>
#include <QTreeView>
#include <QPushButton>

namespace scopy::iiodebugplugin {
class SaveContextSetup : public QWidget
{
	Q_OBJECT
public:
	explicit SaveContextSetup(QTreeView *treeWidget, QWidget *parent = nullptr);
	void setupUi();

private:
	QPushButton *m_saveBtn;
	QPushButton *m_loadBtn;
	QTreeView *m_treeView;
};
} // namespace scopy::iiodebugplugin

#endif // SAVECONTEXTSETUP_H
