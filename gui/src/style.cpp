#include "style.h"
#include "qcolor.h"
#include "qdir.h"
#include "qjsonobject.h"
#include "qwidget.h"

#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <utils.h>
#include <QFileInfo>
#include <QDirIterator>
#include <QFontDatabase>

#include <common/scopyconfig.h>

using namespace scopy;

Style *Style::pinstance_{nullptr};
QJsonDocument *Style::m_global_json{new QJsonDocument()};
QJsonDocument *Style::m_theme_json{new QJsonDocument()};
QMap<QString, QString> *Style::m_styleMap{new QMap<QString, QString>()};

Style::Style(QObject *parent)
	: QObject(parent)
	, m_globalJsonPath("/json/global.json")
	, m_themeJsonPath("/json/Scopy.json")
	, m_qssFolderPath("/qss")
	, m_qssGlobalFile("global")
	, m_m2kqssFile("m2k")
{
	initPaths();
	init();
}

Style::~Style() {}

Style *Style::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new Style(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

QString Style::getStylePath(QString relativePath)
{
	// Check the local plugins folder first
	QString path = config::localStyleFolderPath() + relativePath;
	QDir *pathDir = new QDir(path);
	QFile *pathFile = new QFile(path);

	if(pathDir->exists() || pathFile->exists()) {
		return path;
	}

	path = config::defaultStyleFolderPath() + relativePath;
	pathDir = new QDir(path);
	pathFile = new QFile(path);
	if(pathDir->exists() || pathFile->exists()) {
		return path;
	}

	return "";
}

void Style::initPaths()
{
	m_globalJsonPath = getStylePath(m_globalJsonPath);
	m_themeJsonPath = getStylePath(m_themeJsonPath);
	m_qssFolderPath = getStylePath(m_qssFolderPath);
}

void Style::init(QString theme)
{
	QFontDatabase::addApplicationFont(":/gui/Inter-Regular.ttf");
	QFile global_file = QFile(m_globalJsonPath);
	global_file.open(QIODevice::ReadOnly);
	QByteArray global_data = global_file.readAll();
	global_file.close();
	m_global_json = new QJsonDocument(QJsonDocument::fromJson(global_data));

	if(theme.isEmpty()) {
		theme = getTheme();
	}
	setTheme(theme);
}

void Style::setStyle(QWidget *widget, const char *style, QVariant value, bool force)
{
	style = replaceProperty(style);
	if(!m_styleMap->contains(style)) {
		qCritical("Style: Failed to set style: %s to widget: %s", widget->objectName().toStdString().c_str(),
			  style);
	}

	// set property stylesheet directly to the widget
	// this may be used if the property was overwritten or was not recognized
	if(force) {
		widget->setStyleSheet(widget->styleSheet() + "\n" + m_styleMap->value(style));
	}

	widget->setProperty(style, value);

	// update widget
	widget->style()->unpolish(widget);
	widget->style()->polish(widget);
	QEvent event(QEvent::StyleChange);
	QApplication::sendEvent(widget, &event);
	widget->update();
	widget->updateGeometry();
}

bool Style::isProperty(QString style) { return style.contains("properties"); }

QPixmap Style::getPixmap(QString pixmap, QColor color)
{
	if(color.isValid())
		return Util::ChangeSVGColor(pixmap, color.name(), 1);
	return Util::ChangeSVGColor(pixmap, getAttribute(json::theme::pixmap_color), 1);
}

QString Style::getColorTransparent(const char *key, double transparency)
{
	int percent = (100 - transparency) / 100. * 255.;
	QString color = Style::getAttribute(key);
	QString alpha = QString::number(percent, 16);
	if(alpha.length() == 1) {
		alpha = "0" + alpha;
	}

	color = "#" + alpha + color.right(6);

	return color;
}

QString Style::getTheme() { return QFileInfo(m_themeJsonPath).fileName().replace(".json", ""); }

bool Style::setTheme(QString theme)
{
	QString tmp_theme_path = m_themeJsonPath;
	tmp_theme_path.replace(getTheme() + ".json", theme + ".json");
	if(getTheme() == theme || (QFileInfo(tmp_theme_path).isFile() && tmp_theme_path != m_themeJsonPath)) {
		m_themeJsonPath = tmp_theme_path;

		QFile theme_file = QFile(m_themeJsonPath);
		theme_file.open(QIODevice::ReadOnly);
		QByteArray theme_data = theme_file.readAll();
		theme_file.close();

		m_theme_json = new QJsonDocument(QJsonDocument::fromJson(theme_data));

		genrateStyle();
		return true;
	}

	return false;
}

QStringList Style::getThemeList()
{
	QStringList themes = QStringList();
	QStringList fileList = QDir(QFileInfo(m_themeJsonPath).path()).entryList(QDir::Files);
	QString globalJsonName = QFileInfo(m_globalJsonPath).fileName();

	foreach(QString filename, fileList) {
		if(filename.contains(".json") && filename != globalJsonName) {
			themes.append(filename.replace(".json", ""));
		}
	}

	return themes;
}

QString Style::getAttribute(const char *key)
{
	QString attr = m_theme_json->object().value(key).toString();
	if(attr.isEmpty()) {
		attr = m_global_json->object().value(key).toString();
	}

	return replaceAttributes(attr);
}

QColor Style::getChannelColor(int index) { return getChannelColorList()[index]; }

QList<QColor> Style::getChannelColorList()
{
	QString chString = "ch";
	int index = 0;
	QString color = getAttribute(QString(chString + QString::number(index)).toStdString().c_str());
	QList<QColor> list = QList<QColor>();

	while(!color.isEmpty()) {
		list.append(color);
		color = getAttribute(QString(chString + QString::number(++index)).toStdString().c_str());
	}

	return list;
}

void Style::setBackgroundColor(QWidget *widget, const char *color, bool extend_to_children)
{
	setBackgroundColor(widget, getAttribute(color), extend_to_children);
}

// extending stylesheet to children is not recommended
void Style::setBackgroundColor(QWidget *widget, QString color, bool extend_to_children)
{
	if(extend_to_children) {
		widget->setStyleSheet(widget->styleSheet() + "\nbackground-color: " + color + ";");
	} else {
		widget->setStyleSheet(widget->styleSheet() + "\n.QWidget { background-color: " + color + "; }");
	}
}

QColor Style::getColor(const char *key) { return QColor(getAttribute(key)); }

int Style::getDimension(const char *key) { return getAttribute(key).toInt(); }

const char *Style::replaceProperty(const char *prop)
{
	for(const QString &key : m_theme_json->object().keys()) {
		if(prop == key) {
			QJsonValue value = m_theme_json->object().value(key);
			prop = value.toString().toLocal8Bit().data();
			return prop;
		}
	}

	return prop;
}

QString Style::replaceAttributes(QString style, int calls_limit)
{
	if(style.contains('&') && calls_limit > 0) {
		for(const QString &key : m_theme_json->object().keys()) {
			QJsonValue value = m_theme_json->object().value(key);
			style.replace("&" + key + "&", value.toString());
		}
		for(const QString &key : m_global_json->object().keys()) {
			QJsonValue value = m_global_json->object().value(key);
			style.replace("&" + key + "&", value.toString());
		}

		style = replaceAttributes(style, --calls_limit);
	}
	if(style.contains('&')) {
		qCritical("Style: Failed to replace attribute: %s", style.split('&')[1].toStdString().c_str());
	}

	return style;
}

void Style::genrateStyle()
{
	QDirIterator it(m_qssFolderPath, QStringList() << "*.qss", QDir::Files, QDirIterator::Subdirectories);
	QFile *file;

	while(it.hasNext()) {
		it.next();
		file = new QFile(it.filePath());
		file->open(QIODevice::ReadOnly);
		QString data = QString(file->readAll());
		m_styleMap->insert(QFileInfo(it.filePath()).baseName(), replaceAttributes(data));
	}

	setGlobalStyle();
}

QString Style::getAllProperties()
{
	QString style;
	for(auto it = m_styleMap->keyValueBegin(); it != m_styleMap->keyValueEnd(); ++it) {
		if(isProperty(it->first)) {
			style += it->second + "\n";
		}
	}

	return style;
}

void Style::setGlobalStyle(QWidget *widget)
{
	if(widget) {
		widget->setStyleSheet(m_styleMap->value(m_qssGlobalFile));
	} else {
		qApp->setStyleSheet(getAllProperties() + m_styleMap->value(m_qssGlobalFile));
	}
}

void Style::setM2KStylesheet(QWidget *widget) { widget->setStyleSheet(m_styleMap->value(m_m2kqssFile)); }
