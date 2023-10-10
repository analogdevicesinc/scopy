#include "preferences.h"

#include <QApplication>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_PREFERENCES, "Preferences");

using namespace scopy;

Preferences *Preferences::pinstance_{nullptr};

Preferences::Preferences(QObject *parent)
	: QObject(parent)
{
	connect(parent, SIGNAL(aboutToQuit()), this, SLOT(save()));
}

Preferences::~Preferences() {}

void Preferences::_init(QString k, QVariant v)
{
	if(!p.contains(k)) {
		p.insert(k, v);
	}
	// else - map contains key so it is already initialized to the correct value, nothing to do
}

QVariant Preferences::_get(QString k)
{
	QVariant v = QVariant();
	if(p.contains(k))
		v = p[k];
	return v;
}

void Preferences::set(QString k, QVariant v) { return Preferences::GetInstance()->_set(k, v); }

void Preferences::_set(QString k, QVariant v)
{
	QVariant oldVal = p[k];
	p[k] = v;
	if(oldVal != v)
		Q_EMIT preferenceChanged(k, v);
}

void Preferences::clear() { p.clear(); }

QVariant Preferences::get(QString val) { return Preferences::GetInstance()->_get(val); }

QMap<QString, QVariant> Preferences::getPreferences() const { return p; }

void Preferences::setPreferences(QMap<QString, QVariant> s) { p = s; }

void Preferences::setPreferencesFilename(QString f)
{
	s = new QSettings(f, QSettings::IniFormat, this);
	qInfo(CAT_PREFERENCES) << s->fileName();
}

void Preferences::save()
{
	if(!s)
		return;

	QStringList keys = p.keys();
	for(const QString &key : keys) {
		s->setValue(key, p[key]);
	}

	s->sync();
}

void Preferences::load()
{
	if(!s)
		return;

	QStringList keys = s->allKeys();
	for(const QString &key : keys) {
		p[key] = s->value(key);
	}
}

Preferences *Preferences::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new Preferences(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

void Preferences::init(QString k, QVariant v) { return Preferences::GetInstance()->_init(k, v); }

#include "moc_preferences.cpp"
