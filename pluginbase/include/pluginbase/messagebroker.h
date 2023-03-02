#ifndef MESSAGEBROKER_H
#define MESSAGEBROKER_H

#include <QObject>
#include "scopypluginbase_export.h"
#include <QMap>
#include <QSet>

namespace adiscope {
class SCOPYPLUGINBASE_EXPORT MessageBroker : public QObject
{
	Q_OBJECT
protected:
	MessageBroker(QObject *parent = nullptr);
	~MessageBroker();

public:
	// singleton
	MessageBroker(MessageBroker &other) = delete;
	void operator=(const MessageBroker &) = delete;
	static MessageBroker *GetInstance();

	void subscribe(QObject *obj, QString topic);
	void unsubscribe(QObject *obj, QString topic);
	void publish(QString topic, QString message);
Q_SIGNALS:
	void messageReceived(QString topic, QString message);


private:
	QMap<QObject*, QSet<QString>> map;
	static MessageBroker * pinstance_;

};
}

#endif // MESSAGEBROKER_H
