#include "pluginbase.h"
#include <QLoggingCategory>
#include <QJsonDocument>

Q_LOGGING_CATEGORY(CAT_PLUGIN,"Plugin");

using namespace scopy;

void PluginBase::setParam(QString param, QString category) {
	m_param = param;
	m_category = category;
	m_displayParam = param;
	m_enabled = true;
}

void PluginBase::setEnabled(bool en)
{
	m_enabled = en;
}

void PluginBase::preload() {
}

void PluginBase::postload() {
}

void PluginBase::unload() {

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

bool PluginBase::loadExtraButtons()
{
	return false;
}

void PluginBase::saveSettings(QSettings &s) {

}

void PluginBase::loadSettings(QSettings &s) {

}

void PluginBase::messageCallback(QString topic, QString message)
{

}

void PluginBase::requestTool(QString id)
{
	Q_EMIT requestToolByUuid(ToolMenuEntry::findToolMenuEntryById(m_toolList,id)->uuid());
}

bool PluginBase::enabled() {
	return m_enabled;
}

QString PluginBase::param() {
	return m_param;
}

QString PluginBase::name() {
	return m_name;
}

QString PluginBase::displayName() {
	return m_displayName;
}

QString PluginBase::displayParam() {
	return m_displayParam;
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

QList<QAbstractButton *> PluginBase::extraButtons()
{
	return m_extraButtons;
}

QList<ToolMenuEntry*> PluginBase::toolList() {
	return m_toolList;
}
void PluginBase::showPageCallback() {
	qDebug(CAT_PLUGIN)<<m_name<<"showpage callback";
}
void PluginBase::hidePageCallback() {
	qDebug(CAT_PLUGIN)<<m_name<<"hidepage callback";
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

QString PluginBase::description() {
	return "";
}


void PluginBase::setMetadata(QJsonObject obj)
{
	m_metadata = obj;
}

void PluginBase::initPreferences()
{

}

void PluginBase::init()
{

}

void PluginBase::deinit()
{

}

void PluginBase::cloneExtra(Plugin *)
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
