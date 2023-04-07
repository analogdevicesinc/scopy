#include "toolstack.h"
#include <QDebug>
#include <QLoggingCategory>
#include <QEvent>

using namespace adiscope;

Q_LOGGING_CATEGORY(CAT_TOOLSTACK, "ToolStack")

ToolStack::ToolStack(QWidget *parent) :
	MapStackedWidget(parent)
{
}

ToolStack::~ToolStack()
{
}


void ToolStack::detachTool(QString key) {
	if(isAttached(key)) {
		map[key]->setParent(nullptr);
		show(key);
		Q_EMIT detachSuccesful(key);
	}
}
void ToolStack::attachTool(QString key) {
	if(!isAttached(key)) {
		map[key]->setParent(this);
		addWidget(map[key]);
		show(key);
		Q_EMIT attachSuccesful(key);
	}
}

bool ToolStack::isAttached(QString key) {
	return map[key]->parent()==this;
}

void ToolStack::setAttached(QString key, bool b)
{
	if(b)
		attachTool(key);
	else
		detachTool(key);
}

bool ToolStack::show(QString key)
{
	QWidget *w = map[key];
	if(w)	{
		if(indexOf(map[key]) != -1) {
			setCurrentWidget(w);
			qDebug(CAT_TOOLSTACK)<<key<<" found - showing";
		} else {
			w->show();
			w->raise();
			qDebug(CAT_TOOLSTACK)<<key<<" found in floating stack - showing";
		}
		return true;
	}

	qWarning(CAT_TOOLSTACK)<<key<<"not found in MapStackWidget";
	return false;
}

#include "moc_toolstack.cpp"
