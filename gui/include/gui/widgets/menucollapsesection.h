#ifndef MENUCOLLAPSESECTION_H
#define MENUCOLLAPSESECTION_H

#include <QAbstractButton>
#include <QWidget>
#include <scopy-gui_export.h>
#include <utils.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <stylehelper.h>

namespace scopy {

class SCOPY_GUI_EXPORT MenuCollapseSection : public QWidget {
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	typedef enum {
		MHCW_ARROW,
		MHCW_ONOFF,
		MHCW_NONE
	} MenuHeaderCollapseStyle;

	MenuCollapseSection(QString title,
				 MenuCollapseSection::MenuHeaderCollapseStyle style = MHCW_ARROW,
				 QWidget *parent = nullptr);
	~MenuCollapseSection();

	QAbstractButton *header();
	QVBoxLayout* contentLayout() const;

private:
	QVBoxLayout *m_lay;
	QWidget *m_container;
	QAbstractButton *m_header;
	QVBoxLayout *m_contLayout;
};


class SCOPY_GUI_EXPORT MenuCollapseHeader : public QAbstractButton {
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuCollapseHeader(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style, QWidget *parent);
	~MenuCollapseHeader();
private:
	QAbstractButton *m_ctrl;
	QLabel *m_label;
	QHBoxLayout *lay;

};

}
#endif // MENUCOLLAPSESECTION_H
