#include "style.h"
#include "qcolor.h"
#include "qdir.h"
#include "qjsonobject.h"
#include "qwidget.h"

#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <iostream>
#include <utils.h>
#include <QFileInfo>

using namespace scopy;

Style *Style::pinstance_{nullptr};
QJsonDocument *Style::m_global_json{nullptr};
QJsonDocument *Style::m_theme_json{nullptr};

Style::Style(QObject *parent)
	: QObject(parent)
	, m_globalJsonPath("json/global.json")
	, m_themeJsonPath("json/dark.json")
	, m_qssPath("style.qss")
{
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

void Style::init(QString theme)
{
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

void Style::setStyle(QWidget *widget, const char *style) { widget->setProperty(style, true); }

QPixmap Style::getPixmap(QString pixmap, QColor color)
{
	if(color.isValid())
		return Util::ChangeSVGColor(pixmap, color.name(), 1);
	return Util::ChangeSVGColor(pixmap, getAttribute(json::theme::highlight_color), 1);
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

		applyStyle();
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

	return replaceAttribute(attr);
}

QColor Style::getColor(const char *key) { return QColor(getAttribute(key)); }

int Style::getDimension(const char *key) { return getAttribute(key).toInt(); }

QString Style::replaceAttribute(QString style)
{
	if(style.contains('&')) {
		for(const QString &key : m_theme_json->object().keys()) {
			QJsonValue value = m_theme_json->object().value(key);
			style.replace("&" + key + "&", value.toString());
		}
		for(const QString &key : m_global_json->object().keys()) {
			QJsonValue value = m_global_json->object().value(key);
			style.replace("&" + key + "&", value.toString());
		}
		replaceAttribute(style);
	}

	return style;
}

void Style::applyStyle()
{
	QFile file(m_qssPath);
	file.open(QIODevice::ReadOnly);
	QString style = QString(file.readAll());
	file.close();

	style = replaceAttribute(style);
	qApp->setStyleSheet(style);
	std::cout << style.toStdString() << std::endl;
}
