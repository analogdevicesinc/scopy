#ifndef SCOPY_STATUSMANAGER_H
#define SCOPY_STATUSMANAGER_H

#include <QWidget>
#include <QVariant>
#include <QTimer>

#include "scopy-pluginbase_export.h"
#include "statusmessage.h"

#define NUMBER_OF_RETAINED_MESSAGES 10
#define DEFAULT_DISPLAY_TIME 5000

namespace scopy {
class SCOPY_PLUGINBASE_EXPORT StatusManager : public QObject
{
	Q_OBJECT
protected:
	StatusManager(QObject *parent = nullptr);
	~StatusManager();

public:
	// singleton
	StatusManager(StatusManager &other) = delete;
	void operator=(const StatusManager &) = delete;
	static StatusManager *GetInstance();

	/**
	 * @brief Ads temporary message in the queue to be displayed, when possible, in the Scopy status bar.
	 * @param message QString with the message to be displayed
	 * @param ms The time that the message will be displayed (in milliseconds)
	 * */
	void addTemporaryMessage(const QString &message, int ms = DEFAULT_DISPLAY_TIME);

	/**
	 * @brief Ads temporary QWidget in the queue to be displayed, when possible, in the Scopy status bar. If there
	 * is no display time specified in the "ms" parameter, the creator of the widget is responsible for deleting
	 * it. This will enable StatusManager to send a signal that closes the display, ensuring a smooth transition.
	 * @param message QWidget* to be displayed
	 * @param title QString with the name of the message
	 * @param ms The time that the widget will be displayed (in milliseconds)
	 * */
	void addTemporaryWidget(QWidget *widget, QString title, int ms = -1);

	/**
	 * @brief Overrides any message currently displayed with the message sent as parameter
	 * @param message QString with the urgent message
	 * @param ms The time that the message will be displayed (in milliseconds)
	 * */
	void addUrgentMessage(const QString &message, int ms = DEFAULT_DISPLAY_TIME);

	void setEnabled(bool enabled);
	bool isEnabled() const;

Q_SIGNALS:
	void sendStatus(StatusMessage *);

	void startDisplay(StatusMessage *);
	void clearDisplay();
	void messageAdded();

public Q_SLOTS:
	void processStatusMessage();

private:
	static StatusManager *pinstance_;
	QList<StatusMessage *> *m_itemQueue;
	QTimer *m_timer;
	bool m_enabled;
};
} // namespace scopy

#endif // SCOPY_STATUSMANAGER_H
