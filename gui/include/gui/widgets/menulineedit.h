#ifndef MENULINEEDIT_H
#define MENULINEEDIT_H
#include <QHBoxLayout>
#include <QLineEdit>
#include <QWidget>

#include <scopy-gui_export.h>
#include <stylehelper.h>
#include <utils.h>

namespace scopy {

class SCOPY_GUI_EXPORT MenuLineEdit : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuLineEdit(QWidget *parent = nullptr);
	virtual ~MenuLineEdit();

	QLineEdit *edit();
	void applyStylesheet();

private:
	QLineEdit *m_edit;
};
} // namespace scopy

#endif // MENULINEEDIT_H
