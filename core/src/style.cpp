#include "style.h"
#include "qcolor.h"
#include "qjsonobject.h"
#include "qwidget.h"

#include <QApplication>
#include <QFile>
#include <QJsonDocument>

using namespace scopy;

QJsonDocument *Style::m_json{new QJsonDocument()};

Style::Style(QObject *parent)
	: QObject(parent)
	, m_jsonPath("style_variables.json")
	, m_qssPath("style.qss")
{}

Style::~Style() {}

void Style::init()
{
	QFile file(m_jsonPath);
	file.open(QIODevice::ReadOnly);
	QByteArray data = file.readAll();
	file.close();
	m_json = new QJsonDocument(QJsonDocument::fromJson(data));

	applyStyle();
}

void Style::setStyle(QWidget *widget, char *style) { widget->setProperty(style, true); }

QString Style::getAttribute(char *key) { return m_json->object().value(key).toString(); }

QColor Style::getColor(char *key) { return QColor(getAttribute(key)); }

int Style::getDimension(char *key) { return getAttribute(key).toInt(); }

void Style::applyStyle()
{
	QFile file(m_qssPath);
	file.open(QIODevice::ReadOnly);
	QString style = QString(file.readAll());
	file.close();

	for(const QString &key : m_json->object().keys()) {
		QJsonValue value = m_json->object().value(key);
		style.replace("&" + key + "&", value.toString());
	}

	qApp->setStyleSheet(style);
}
