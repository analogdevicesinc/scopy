#include "plugin/baseplugin.h"
#include "logging_categories.h"

using namespace adiscope;
BasePlugin::BasePlugin(QWidget *parent, bool dockable) : parent(parent), dockable(dockable)
{
//	init();
}

BasePlugin::~BasePlugin()
{

}

void BasePlugin::init()
{
	qDebug(CAT_BASEPLUGIN) << "init()";
}
