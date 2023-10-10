#ifndef SCOPY_STATUSMANAGER_H
#define SCOPY_STATUSMANAGER_H

#include <QWidget>
#include <QVariant>

#include "scopy-pluginbase_export.h"

#define NO_RETAINED_MESSAGES 10
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
	 * @param ms The time that the widget will be displayed (in milliseconds)
	 * */
	void addTemporaryMessage(const QString &message, int ms = DEFAULT_DISPLAY_TIME);

	/**
	 * @brief Ads temporary QWidget in the queue to be displayed, when possible, in the Scopy status bar.
	 * @param message QWidget* to be displayed
	 * @param ms The time that the widget will be displayed (in milliseconds)
	 * */
	void addTemporaryWidget(QWidget *widget, int ms = DEFAULT_DISPLAY_TIME);

Q_SIGNALS:
	void announceStatusAvailable();
	void sendStatus(QVariant status, int ms);

public Q_SLOTS:
	void requestStatus();

private:
	static StatusManager *pinstance_;
	QList<QPair<QVariant, int>> m_itemQueue;
};
} // namespace scopy

#endif // SCOPY_STATUSMANAGER_H
