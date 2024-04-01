#ifndef EDITTEXTMENUHEADER_H
#define EDITTEXTMENUHEADER_H

#include <QWidget>
#include <QFrame>
#include <QLineEdit>

#include <utils.h>
#include <scopy-gui_export.h>

namespace scopy {
class SCOPY_GUI_EXPORT EditTextMenuHeader : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	explicit EditTextMenuHeader(QString title, QPen pen, QWidget *parent = nullptr);
	~EditTextMenuHeader();

	QLineEdit *lineEdit() const;
	void applyStylesheet();

private:
	QFrame *m_line;
	QPen m_pen;
	QLineEdit *m_lineEdit;
};
} // namespace scopy
#endif // EDITTEXTMENUHEADER_H
