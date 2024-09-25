#include "iiowidgetselector.h"

#include <QVBoxLayout>
#include <gui/stylehelper.h>

using namespace scopy;

IIOWidgetSelector::IIOWidgetSelector(IIOItem *root, QWidget *parent)
	: QFrame(parent)
{
	setLayout(new QVBoxLayout(this));
	layout()->setContentsMargins(0, 0, 0, 0);
	m_treeView = new QTreeView(this);
	m_model = new QStandardItemModel(m_treeView);
	m_model->appendRow(root);
	m_treeView->setModel(m_model);
	m_treeView->expand(m_model->index(0, 0));
	m_treeView->setHeaderHidden(true);
	layout()->addWidget(m_treeView);
	StyleHelper::TreeViewDebugger(m_treeView, "TreeView");
	setStyleSheet("background-color: " + StyleHelper::getColor("ScopyBackground"));

	connect(m_treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
		&IIOWidgetSelector::itemSelectionChanged);
}

IIOWidgetSelector::~IIOWidgetSelector()
{
	// This should be done so that the root item is not deleted when the model is deleted
	m_model->takeRow(0);
}

QTreeView *IIOWidgetSelector::getTree() const { return m_treeView; }

QStandardItemModel *IIOWidgetSelector::getModel() const { return m_model; }

#include "moc_iiowidgetselector.cpp"
