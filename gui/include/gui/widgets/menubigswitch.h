#ifndef MENUBIGSWITCH_H
#define MENUBIGSWITCH_H

#include <QHBoxLayout>
#include <QWidget>

#include <customSwitch.h>
#include <scopy-gui_export.h>
#include <stylehelper.h>
#include <utils.h>

namespace scopy {

class SCOPY_GUI_EXPORT MenuBigSwitch : public QWidget
{
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
} // namespace scopy
#endif // MENUBIGSWITCH_H
