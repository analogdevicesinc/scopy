#ifndef SCOPY_SCOPYSTATUSBAR_H
#define SCOPY_SCOPYSTATUSBAR_H

#include <QObject>
#include <QStatusBar>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QList>
#include "hoverwidget.h"
#include "pluginbase/statusmessage.h"
#include "utils.h"
#include "menu_anim.hpp"
#include "scopy-gui_export.h"

namespace scopy {
class SCOPY_GUI_EXPORT ScopyStatusBar : public MenuVAnim
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	explicit ScopyStatusBar(QWidget *parent = nullptr);

Q_SIGNALS:
	void requestHistory();

public Q_SLOTS:
	void displayStatusMessage(StatusMessage *statusMessage);
	void clearStatusMessage();

private:
	void initUi();

	void addToRight(QWidget *widget);
	void addToLeft(QWidget *widget);

	StatusMessage *m_message;

	// UI elements
	QWidget *m_leftWidget;
	QWidget *m_rightWidget;
};
} // namespace scopy

#endif // SCOPY_SCOPYSTATUSBAR_H
