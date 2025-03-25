/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

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
#include <QLoggingCategory>

#include <common/scopyconfig.h>

Q_LOGGING_CATEGORY(CAT_STYLE, "STYLE");

using namespace scopy;

Style *Style::pinstance_{nullptr};
QJsonDocument *Style::m_global_json{new QJsonDocument()};
QJsonDocument *Style::m_theme_json{new QJsonDocument()};
QMap<QString, QString> *Style::m_styleMap{new QMap<QString, QString>()};
QFileInfoList Style::m_pkgThemes{};
QFileInfoList Style::m_pkgQss{};

Style::Style(QObject *parent)
	: QObject(parent)
	, m_globalJsonPath("/json/global.json")
	, m_themeJsonPath("/json/Scopy.json")
	, m_jsonFolderPath("/json")
	, m_qssFolderPath("/qss")
	, m_qssGlobalFile("global")
	, m_m2kqssFile("m2k")
{
	initPaths();
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
	m_jsonFolderPath = getStylePath(m_jsonFolderPath);
}

bool Style::init(QString theme, float fontScale)
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

	return setTheme(getThemePath(theme), fontScale);
}

QString Style::getThemePath(QString theme)
{
	QString tmp_theme_path = m_themeJsonPath;
	tmp_theme_path.replace(getTheme() + ".json", theme + ".json");
	if(!QFile::exists(tmp_theme_path)) {
		tmp_theme_path = getThemeFromPkgs(theme);
		qInfo(CAT_STYLE) << "Theme not found in the default path, searching in the packages folder...";
	}

	return tmp_theme_path;
}

void Style::setStyle(QWidget *widget, const char *style, QVariant value, bool force)
{
	style = replaceProperty(style);
	if(!m_styleMap->contains(style)) {
		qCritical(CAT_STYLE) << "Style: Failed to set style: " << widget->objectName().toStdString().c_str()
				     << " to widget: " << style;
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

QString Style::getTheme() { return QFileInfo(m_themeJsonPath).baseName(); }

bool Style::setTheme(QString themePath, float fontScale)
{
	bool ret = true;

	if(QFileInfo(themePath).isFile()) {
		m_themeJsonPath = themePath;
		QFile theme_file = QFile(m_themeJsonPath);
		theme_file.open(QIODevice::ReadOnly);
		QByteArray theme_data = theme_file.readAll();
		theme_file.close();

		m_theme_json = new QJsonDocument(QJsonDocument::fromJson(theme_data));

		adjustJsonForScaling(fontScale);
		generateStyle();
		QIcon::setThemeName(getAttribute(json::theme::icon_theme_folder));
	} else {
		qCritical(CAT_STYLE) << "Style: Failed set theme: " << themePath.toStdString().c_str();
		ret = false;
	}

	return ret;
}

QString Style::getThemeFromPkgs(QString theme)
{
	QString path = "";
	for(const QFileInfo &file : qAsConst(m_pkgThemes)) {
		if(file.baseName().contains(theme)) {
			path = file.filePath();
			break;
		}
	}
	return path;
}

QStringList Style::getThemeList()
{
	QStringList themes = QStringList();
	QStringList fileList;
	std::transform(m_pkgThemes.begin(), m_pkgThemes.end(), std::back_inserter(fileList),
		       [](const QFileInfo &fi) { return fi.fileName(); });
	fileList.append(QDir(m_jsonFolderPath).entryList(QDir::Files));
	QString globalJsonName = QFileInfo(m_globalJsonPath).fileName();

	foreach(QString filename, fileList) {
		if(filename.contains(".json") && filename != globalJsonName) {
			themes.append(filename.replace(".json", ""));
		}
	}

	return themes;
}

void Style::adjustJsonForScaling(float scale)
{
	if(scale != 1) {
		for(QJsonDocument *jsonDoc : {m_theme_json, m_global_json}) {
			QJsonObject jsonObj = jsonDoc->object();
			for(QString key : jsonObj.keys()) {
				QJsonValue value = jsonObj.value(key);
				jsonObj[key] = adjustForScaling(key, value.toString(), scale);
			}
			jsonDoc->setObject(jsonObj);
		}
	}
}

QString Style::getAttribute(const char *key)
{
	QString attr = m_theme_json->object().value(key).toString();
	if(attr.isEmpty()) {
		attr = m_global_json->object().value(key).toString();
	}

	return replaceAttributes(attr);
}

QColor Style::getChannelColor(int index)
{
	QList<QColor> list = getChannelColorList();
	if(list.count() <= index)
		return QColor();

	return getChannelColorList()[index];
}

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

// using setStyle is more efficient if possible
// for example if the color is background_primary, you may use style::properties::widget::basicBackground
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

int Style::getDimension(const char *key)
{
	QString attr = getAttribute(key);
	int number = attr.toInt();

	// this is for attributes with a string suffix. like "10px"
	if(number == 0) {
		QRegularExpression regex("(\\d+)");
		QRegularExpressionMatch match = regex.match(attr);

		if(match.hasMatch()) {
			QString numberStr = match.captured(1);
			number = numberStr.toInt();
		}
	}

	return number;
}

const char *Style::replaceProperty(const char *prop)
{
	for(const QString &key : m_theme_json->object().keys()) {
		if(prop == key) {
			QJsonValue value = m_theme_json->object().value(key);
			prop = strdup(value.toString().toLocal8Bit().data());
			return prop;
		}
	}

	return prop;
}

QString Style::replaceAttributes(QString style, int calls_limit)
{
	if(style.contains('&') && calls_limit > 0) {
		for(const QString &key : m_theme_json->object().keys()) {
			QString value = m_theme_json->object().value(key).toString();
			style.replace("&" + key + "&", value);
		}
		for(const QString &key : m_global_json->object().keys()) {
			QString value = m_global_json->object().value(key).toString();
			style.replace("&" + key + "&", value);
		}

		style = replaceAttributes(style, --calls_limit);
	}
	if(style.contains('&')) {
		qCritical(CAT_STYLE) << "Style: Failed to replace attribute: "
				     << style.split('&')[1].toStdString().c_str();
	}

	return style;
}

void Style::setPkgsThemes(QFileInfoList infoList) { m_pkgThemes = infoList; }

void Style::setPkgsQss(QFileInfoList infoList) { m_pkgQss = infoList; }

void Style::generateStyle()
{
	QFileInfoList qssList = getQssList(m_qssFolderPath);
	qssList.append(m_pkgQss);
	QFile *file;
	for(const QFileInfo &fInfo : qAsConst(qssList)) {
		file = new QFile(fInfo.filePath());
		file->open(QIODevice::ReadOnly);
		QString data = QString(file->readAll());
		m_styleMap->insert(fInfo.baseName(), replaceAttributes(data));
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

QFileInfoList Style::getQssList(QString path)
{
	QDirIterator it(path, QStringList() << "*.qss", QDir::Files, QDirIterator::Subdirectories);
	QFileInfoList files;
	while(it.hasNext()) {
		it.next();
		files.append(it.fileInfo());
	}
	return files;
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

QString Style::scaleNumberInString(QString string, float factor)
{
	QRegularExpression regex("(\\d+)");
	QRegularExpressionMatch match = regex.match(string);

	if(match.hasMatch()) {
		QString numberStr = match.captured(1);
		int number = numberStr.toInt();
		int scaledNumber = number * factor;
		return string.replace(regex, QString::number(scaledNumber));
	}

	return string;
}

QString Style::adjustForScaling(QString key, QString value, float scale)
{
	if(QString(key).startsWith("font_size")) {
		value = scaleNumberInString(value, scale);
	}
	if(QString(key).startsWith("unit_")) {
		value = scaleNumberInString(value, (scale - 1) / 2 + 1);
	}

	return value;
}
