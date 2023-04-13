#include "messagebroker.h"
#include <QApplication>


using namespace scopy;

MessageBroker* MessageBroker::pinstance_{nullptr};

MessageBroker::MessageBroker(QObject *parent) : QObject(parent)
{

}

MessageBroker::~MessageBroker()
{

}


MessageBroker *MessageBroker::GetInstance()
{
	if (pinstance_ == nullptr)
	{
		pinstance_ = new MessageBroker(QApplication::instance()); // singleton has the app as parent
	}
	return pinstance_;
}

void MessageBroker::subscribe(QObject *obj, QString topic)
{
	if(!map.contains(obj))
		map.insert(obj,{"broadcast"});
	map[obj].insert(topic);
}

void MessageBroker::unsubscribe(QObject *obj, QString topic)
{
	if(map.contains(obj)) {
		map[obj].remove(topic);
		if(map[obj].count() == 0) {
			map.remove(obj);
		}
	}

}

void MessageBroker::publish(QString topic, QString message)
{

	bool processed;
	QList<QObject*> keys = map.keys();
	for(auto &&k : keys) {
		if(map.value(k).contains(topic)) {
			QMetaObject::invokeMethod(k, "messageCallback",
						  Qt::AutoConnection,
//						  Q_RETURN_ARG(bool, processed),
						  Q_ARG(QString, topic),
						  Q_ARG(QString, message));
		}
	}
}


#include "moc_messagebroker.cpp"
