#include "pluginbase.h"
#include <QLoggingCategory>
#include <QJsonDocument>

Q_LOGGING_CATEGORY(CAT_PLUGIN,"Plugin");

using namespace adiscope;

void PluginBase::setUri(QString uri) {
	m_uri = uri;
}

void PluginBase::preload() {
}

void PluginBase::postload() {
}

void PluginBase::unload() {

}

void PluginBase::loadApi()
{

}

bool PluginBase::loadIcon() {
	m_icon = nullptr;
	return false;
}

bool PluginBase::loadPage(){
	m_page = nullptr;
	return false;
}

void PluginBase::loadToolList()	{
}

bool PluginBase::loadPreferencesPage()
{
	m_preferencesPage = nullptr;
	return false;
}

void PluginBase::saveSettings()
{
	if(pluginApi)
		pluginApi->save();
}

void PluginBase::loadSettings()
{
	if(pluginApi)
		pluginApi->load();
}

void PluginBase::saveSettings(QSettings& s)
{
	if(pluginApi)
		pluginApi->save(s);
}

void PluginBase::loadSettings(QSettings& s)
{
	if(pluginApi)
		pluginApi->load(s);
}

ApiObject *PluginBase::api()
{
	return pluginApi;
}

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

QWidget *PluginBase::preferencesPage()
{
	return m_preferencesPage;
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

QJsonObject PluginBase::metadata() {
	return m_metadata;
}

QString PluginBase::about()
{
	return "";
}

QString PluginBase::version()
{
	return "";
}

void PluginBase::setMetadata(QJsonObject obj)
{
	m_metadata = obj;
}

void PluginBase::initPreferences()
{

}

void PluginBase::initMetadata() {
	loadMetadata(

R"plugin(
{
   "priority":1,
}
)plugin");

}

void PluginBase::loadMetadata(QString data) {
	QJsonParseError err;
	QJsonDocument doc = QJsonDocument::fromJson(data.toUtf8(), &err);
	if(err.error != QJsonParseError::NoError) {
		qCritical(CAT_PLUGIN) << m_name <<" plugin - JSON Parse error !" << err.errorString();
		qCritical(CAT_PLUGIN) << data;
		qCritical(CAT_PLUGIN) << QString(" ").repeated(err.offset)+"^";
	}
	m_metadata = doc.object();
}
