#include "scopyenvironment.h"
#include <QApplication>

using namespace scopy;
ScopyEnvironment* ScopyEnvironment::pinstance_{nullptr};

ScopyEnvironment::ScopyEnvironment(QObject *parent) : QObject(parent)
{

}

ScopyEnvironment::~ScopyEnvironment()
{

}

QVariant ScopyEnvironment::get(QString k)
{
	QVariant v = QVariant();
	if(p.contains(k))
		v = p[k];
	return v;
}

void ScopyEnvironment::set(QString k, QVariant v)
{
	p[k] = v;
}

ScopyEnvironment *ScopyEnvironment::GetInstance()
{
	if (pinstance_ == nullptr) 	{
		pinstance_ = new ScopyEnvironment(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

#include "moc_scopyenvironment.cpp"
