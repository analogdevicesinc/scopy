#include "toolstack.h"
#include <QDebug>
#include <QLoggingCategory>

using namespace adiscope;

Q_LOGGING_CATEGORY(CAT_TOOLSTACK, "ToolStack")

ToolStack::ToolStack(QWidget *parent) :
	MapStackedWidget(parent)
{
}

ToolStack::~ToolStack()
{
}

void ToolStack::detachTool(QString tool) {
	map[tool]->setParent(nullptr);
	show(tool);
	Q_EMIT detachSuccesful(tool);
}
void ToolStack::attachTool(QString tool) {
	map[tool]->setParent(this);
	addWidget(map[tool]);
	show(tool);
	Q_EMIT attachSuccesful(tool);
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
