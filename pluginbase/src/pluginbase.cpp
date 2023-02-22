#include "pluginbase.h"
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_PLUGIN,"Plugin");

using namespace adiscope;
QString PluginBase::uri() {
	return m_uri;
}
QString PluginBase::name() {
	return m_name;
}
QWidget* PluginBase::icon() {
	return m_icon;
}
QWidget* PluginBase::page() {
	return m_page;
}
QList<ToolMenuEntry*> PluginBase::toolList() {
	return m_toolList;
}
void PluginBase::showPageCallback() {
	qDebug(CAT_PLUGIN)<<m_uri<<"showpage callback";
}
void PluginBase::hidePageCallback() {
	qDebug(CAT_PLUGIN)<<m_uri<<"hidepage callback";
}

int PluginBase::priority() {
	return 0;
}
