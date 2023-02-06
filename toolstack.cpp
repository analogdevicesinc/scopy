#include "toolstack.h"
#include <QDebug>

using namespace adiscope;
ToolStack::ToolStack(QWidget *parent) :
	QStackedWidget(parent)
{

}

void ToolStack::addTool(QString tool, QWidget* w) {
	map[tool] = w;
	addWidget(w);
}
void ToolStack::showTool(QString tool)
{
	QWidget *w = map[tool];
	if(w)	{
		if(indexOf(map[tool]) != -1) {
			setCurrentWidget(w);
		} else {
			w->show();
			w->raise();
		}

	} else {
		qDebug()<<"WARNING"<<tool<<" not found in ToolStack";
	}

}

void ToolStack::removeTool(QString tool)
{
	if(indexOf(map[tool]) != -1) { // widget is in stack
		removeWidget(map[tool]);
	}
	map.remove(tool);

}
void ToolStack::detachTool(QString tool) {
	map[tool]->setParent(nullptr);
	showTool(tool);
	Q_EMIT detachSuccesful(tool);
}
void ToolStack::attachTool(QString tool) {
	map[tool]->setParent(this);
	addWidget(map[tool]);
	showTool(tool);
	Q_EMIT attachSuccesful(tool);
}

ToolStack::~ToolStack()
{
}
