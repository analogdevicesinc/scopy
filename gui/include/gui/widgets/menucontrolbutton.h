#ifndef MENUCONTROLBUTTON_H
#define MENUCONTROLBUTTON_H

#include "scopy-gui_export.h"
#include <QWidget>
#include <QAbstractButton>
#include <utils.h>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

namespace scopy {
    class SCOPY_GUI_EXPORT MenuControlButton : public QAbstractButton {
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	typedef enum {
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

	QCheckBox* checkBox();
	QPushButton* button();

private:
	void applyStylesheet();

	int width;
	QColor m_color;
	QHBoxLayout *lay;
	QCheckBox *m_chk;
	QLabel *m_label;
	QPushButton *m_btn;
	CheckboxStyle m_cs;
};



class SCOPY_GUI_EXPORT CollapsableMenuControlButton : public QWidget {
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

}
#endif // MENUCONTROLBUTTON_H
