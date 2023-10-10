#ifndef MENUCONTROLBUTTON_H
#define MENUCONTROLBUTTON_H

#include "scopy-gui_export.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include <utils.h>

namespace scopy {
class SCOPY_GUI_EXPORT MenuControlButton : public QAbstractButton
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	typedef enum
	{
		CS_CIRCLE,
		CS_SQUARE,
		CS_BLUESQUARE,
		CS_COLLAPSE,
	} CheckboxStyle;

	MenuControlButton(QWidget *parent = nullptr);
	~MenuControlButton();

	void setColor(QColor c);
	void setCheckBoxStyle(MenuControlButton::CheckboxStyle cs);
	void setName(QString s);

	void setDoubleClickToOpenMenu(bool b);
	void setOpenMenuChecksThis(bool b);

	QCheckBox *checkBox();
	QPushButton *button();

Q_SIGNALS:
	void doubleClicked();

private:
	void applyStylesheet();

	int width;
	QColor m_color;
	QHBoxLayout *lay;
	QCheckBox *m_chk;
	QLabel *m_label;
	QPushButton *m_btn;
	CheckboxStyle m_cs;
	QMetaObject::Connection dblClickToOpenMenu;
	QMetaObject::Connection openMenuChecksThis;

protected:
	void mouseDoubleClickEvent(QMouseEvent *event) override;
	void mousePressEvent(QMouseEvent *event) override;
};

class SCOPY_GUI_EXPORT CollapsableMenuControlButton : public QWidget
{
	Q_OBJECT
public:
	CollapsableMenuControlButton(QWidget *parent = nullptr);
	~CollapsableMenuControlButton();

	void add(QWidget *ch);
	MenuControlButton *getControlBtn();

private:
	MenuControlButton *m_ctrl;
	QWidget *m_container;
	QVBoxLayout *m_contLayout;
	QVBoxLayout *m_lay;
};

} // namespace scopy
#endif // MENUCONTROLBUTTON_H
