#ifndef MENUHEADER_H
#define MENUHEADER_H

#include <scopy-gui_export.h>
#include <QWidget>
#include <utils.h>
#include <QLabel>

namespace scopy {
class SCOPY_GUI_EXPORT MenuHeaderWidget : public QWidget {
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuHeaderWidget(QString title, QPen pen, QWidget *parent = nullptr);
	~MenuHeaderWidget();

	QLabel *label();
	void applyStylesheet();

private:
	QLabel *m_label;
	QFrame *m_line;
	QPen m_pen;
};
}

#endif // MENUHEADER_H
