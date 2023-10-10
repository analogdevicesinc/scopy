#ifndef MENUHEADER_H
#define MENUHEADER_H

#include <QLabel>
#include <QWidget>

#include <scopy-gui_export.h>
#include <utils.h>

namespace scopy {
class SCOPY_GUI_EXPORT MenuHeaderWidget : public QWidget
{
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
} // namespace scopy

#endif // MENUHEADER_H
