#include "savecontextsetup.h"
#include <gui/stylehelper.h>
#include <QHBoxLayout>

using namespace scopy::debugger;

SaveContextSetup::SaveContextSetup(QTreeView *treeWidget, QWidget *parent)
	: QWidget(parent)
	, m_saveBtn(new QPushButton("Save", this))
	, m_loadBtn(new QPushButton("Load", this))
{
	setupUi();
}

void SaveContextSetup::setupUi()
{
	setLayout(new QHBoxLayout(this));

	StyleHelper::BasicButton(m_saveBtn, "SaveContextSetupButton");
	StyleHelper::BasicButton(m_loadBtn, "LoadContextSetupButton");

	layout()->addWidget(m_saveBtn);
	layout()->addWidget(m_loadBtn);
}

#include "moc_savecontextsetup.cpp"
