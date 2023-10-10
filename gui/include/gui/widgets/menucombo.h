#ifndef MENUCOMBO_H
#define MENUCOMBO_H

#include <QComboBox>
#include <QLabel>
#include <QPen>
#include <QWidget>

#include <scopy-gui_export.h>
#include <utils.h>

namespace scopy {

class SCOPY_GUI_EXPORT MenuCombo : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuCombo(QString title, QWidget *parent = nullptr);
	virtual ~MenuCombo();

	QComboBox *combo();
	void applyStylesheet();

private:
	QLabel *m_label;
	QComboBox *m_combo;
};

class SCOPY_GUI_EXPORT MenuComboWidget : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuComboWidget(QString title, QWidget *parent = nullptr);
	virtual ~MenuComboWidget();

	QComboBox *combo();
	void applyStylesheet();

private:
	MenuCombo *m_combo;
	QPen m_pen;
};
} // namespace scopy

#endif // MENUCOMBO_H
