#ifndef MENULINEEDIT_H
#define MENULINEEDIT_H
#include <scopy-gui_export.h>
#include <QWidget>
#include <utils.h>
#include <QHBoxLayout>
#include <QLineEdit>
#include <stylehelper.h>

namespace scopy {

class SCOPY_GUI_EXPORT MenuLineEdit : public QWidget {
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
}

#endif // MENULINEEDIT_H
