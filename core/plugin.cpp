#include <plugin.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_PLUGIN,"Plugin");

using namespace adiscope;
QString Plugin::uri() {
	return m_uri;
}
QString Plugin::name() {
	return m_name;
}
QWidget* Plugin::icon() {
	return m_icon;
}
QWidget* Plugin::page() {
	return m_page;
}
QList<ToolMenuEntry> Plugin::toolList() {
	return m_toolList;
}
void Plugin::showPageCallback() {
	qDebug(CAT_PLUGIN)<<m_uri<<"showpage callback";
}
void Plugin::hidePageCallback() {
	qDebug(CAT_PLUGIN)<<m_uri<<"hidepage callback";
}

