#ifndef MENUCOLLAPSESECTION_H
#define MENUCOLLAPSESECTION_H

#include <QAbstractButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QWidget>
#include <baseheader.h>

#include <scopy-gui_export.h>
#include <stylehelper.h>
#include <utils.h>

namespace scopy {
class MenuCollapseHeader;

class SCOPY_GUI_EXPORT MenuCollapseSection : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	typedef enum
	{
		MHCW_ARROW,
		MHCW_ONOFF,
		MHCW_NONE
	} MenuHeaderCollapseStyle;

	typedef enum
	{
		MHW_BASEWIDGET,
		MHW_COMPOSITEWIDGET
	} MenuHeaderWidgetType;

	MenuCollapseSection(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style = MHCW_ARROW,
			    MenuCollapseSection::MenuHeaderWidgetType headerType = MHW_BASEWIDGET,
			    QWidget *parent = nullptr);
	~MenuCollapseSection();

	QAbstractButton *header();
	QVBoxLayout *contentLayout() const;
	QString title();
	void setTitle(QString s);

private:
	QString m_title;
	QVBoxLayout *m_lay;
	QWidget *m_container;
	MenuCollapseHeader *m_header;
	QVBoxLayout *m_contLayout;
};

class SCOPY_GUI_EXPORT MenuCollapseHeader : public QAbstractButton
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuCollapseHeader(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style,
			   MenuCollapseSection::MenuHeaderWidgetType headerType, QWidget *parent);
	~MenuCollapseHeader();
	QString title();

	QWidget *headerWidget() const;

private:
	QAbstractButton *m_ctrl;
	QWidget *m_headerWidget;
	QHBoxLayout *lay;
};

} // namespace scopy
#endif // MENUCOLLAPSESECTION_H
