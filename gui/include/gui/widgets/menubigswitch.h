#ifndef MENUBIGSWITCH_H
#define MENUBIGSWITCH_H

#include <scopy-gui_export.h>
#include <QWidget>
#include <utils.h>
#include <QHBoxLayout>
#include <customSwitch.h>
#include <stylehelper.h>

namespace scopy {

class SCOPY_GUI_EXPORT MenuBigSwitch : public QWidget {
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuBigSwitch(QString on, QString off, QWidget *parent = nullptr);
	virtual ~MenuBigSwitch();

	CustomSwitch *onOffswitch();
	void applyStylesheet();

private:
	CustomSwitch *m_switch;
};
}
#endif // MENUBIGSWITCH_H
