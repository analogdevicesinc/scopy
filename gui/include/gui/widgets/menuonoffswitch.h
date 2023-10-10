#ifndef MENUONOFFSWITCH_H
#define MENUONOFFSWITCH_H

#include <QAbstractButton>
#include <QHBoxLayout>
#include <QWidget>

#include <scopy-gui_export.h>
#include <stylehelper.h>
#include <utils.h>

namespace scopy {

class SCOPY_GUI_EXPORT MenuOnOffSwitch : public QWidget
{
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
} // namespace scopy

#endif // MENUONOFFSWITCH_H
