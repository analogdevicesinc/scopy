#ifndef MENUONOFFSWITCH_H
#define MENUONOFFSWITCH_H

#include <scopy-gui_export.h>
#include <QWidget>
#include <utils.h>
#include <QHBoxLayout>
#include <QAbstractButton>
#include <stylehelper.h>

namespace scopy {

class SCOPY_GUI_EXPORT MenuOnOffSwitch : public QWidget {
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuOnOffSwitch(QString title, QWidget *parent = nullptr, bool medium = false);
	virtual ~MenuOnOffSwitch();

	QAbstractButton *onOffswitch();
	void applyStylesheet();

private:
	QLabel *m_label;
	QAbstractButton *m_switch;
};
}

#endif // MENUONOFFSWITCH_H
