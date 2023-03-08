#ifndef MESSAGEBROKER_H
#define MESSAGEBROKER_H

#include <QObject>
#include "scopypluginbase_export.h"
#include <QMap>
#include <QSet>

namespace adiscope {
/**
 * @brief The MessageBroker class
 * MessageBroker is a singleton class to implement a publisher-subscriber message exchange
 *
 */
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

	/**
	 * @brief subscribe
	 * @param obj
	 * @param topic
	 * subscribe object to a topic
	 * when the topic is published, the object will receive the message
	 */
	void subscribe(QObject *obj, QString topic);

	/**
	 * @brief unsubscribe
	 * @param obj
	 * @param topic
	 * unsubscribe object from the topic
	 */
	void unsubscribe(QObject *obj, QString topic);

	/**
	 * @brief publish
	 * @param topic
	 * @param message
	 * publish a topic with a message
	 */
	void publish(QString topic, QString message);

private:
	QMap<QObject*, QSet<QString>> map;
	static MessageBroker * pinstance_;

};
}

#endif // MESSAGEBROKER_H
