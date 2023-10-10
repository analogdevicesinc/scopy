#ifndef SCOPY_SCOPYSTATUSBAR_H
#define SCOPY_SCOPYSTATUSBAR_H

#include <QObject>
#include <QStatusBar>
#include <QPushButton>
#include <QListWidget>
#include "hoverwidget.h"
#include "scopy-gui_export.h"

namespace scopy {
class SCOPY_GUI_EXPORT ScopyStatusBar : public QStatusBar
{
	Q_OBJECT
public:
	explicit ScopyStatusBar(QWidget *parent = nullptr);

public Q_SLOTS:
	/**
	 * @brief Verifies weather ScopyStatusBar can display a new status. It can display it if the current status
	 * is empty.
	 * */
	void shouldDisplayNewStatus();

	/**
	 * @brief Displays the received status.
	 * @param status A QString or a QWidget* that will be processed and displayed.
	 * @param ms The time the status will appear on screen, in milliseconds.
	 * */
	void processStatus(QVariant status, int ms);

	/**
	 * @brief Pops up the history of the last statuses.
	 * */
	void showHistory(bool checked);

	/**
	 * @brief overrides the current message with the one send as parameter.
	 * @param message QString with the urgent message
	 * @param ms The time that the message will be displayed (in milliseconds)
	 * */
	void receiveUrgentMessage(QString message, int ms);

private:
	void initUi();
	void initHistory();

	QPushButton *m_historyButton;
	QListWidget *m_historyList;
	HoverWidget *m_hoverWidget;
};
} // namespace scopy

#endif // SCOPY_SCOPYSTATUSBAR_H
