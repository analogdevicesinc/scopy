#include "toolstack.h"
#include <QDebug>
#include <QLoggingCategory>
#include <QEvent>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_TOOLSTACK, "ToolStack")

ToolStack::ToolStack(QWidget *parent) :
	MapStackedWidget(parent)
{
}

ToolStack::~ToolStack()
{
}


#include "moc_toolstack.cpp"
