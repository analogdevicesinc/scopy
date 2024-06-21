#ifndef MENUSECTIONWIDGET_H
#define MENUSECTIONWIDGET_H
#include "compositewidget.h"
#include "menucollapsesection.h"
#include <QScrollArea>
#include <QVBoxLayout>
#include <QWidget>

#include <scopy-gui_export.h>
#include <utils.h>

namespace scopy {

class SCOPY_GUI_EXPORT MenuSectionWidget : public QWidget
{
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuSectionWidget(QWidget *parent = nullptr);
	~MenuSectionWidget();
	QVBoxLayout *contentLayout() const;

private:
	QVBoxLayout *m_layout;
};

class SCOPY_GUI_EXPORT MenuVScrollArea : public QScrollArea
{
	Q_OBJECT
	//	QWIDGET_PAINT_EVENT_HELPER
public:
	MenuVScrollArea(QWidget *parent = nullptr);
	~MenuVScrollArea();
	QVBoxLayout *contentLayout() const;

private:
	QVBoxLayout *m_layout;
};

class SCOPY_GUI_EXPORT MenuSectionCollapseWidget : public QWidget, public Collapsable, public CompositeWidget {
	Q_OBJECT
	QWIDGET_PAINT_EVENT_HELPER
public:

	MenuSectionCollapseWidget(QString title, MenuCollapseSection::MenuHeaderCollapseStyle style, QWidget *parent = nullptr);
	~MenuSectionCollapseWidget();
	QVBoxLayout *contentLayout() const;

	void add(QWidget *w) override;
	void remove(QWidget *w) override;

	bool collapsed() override;
	void setCollapsed(bool b) override;
	MenuCollapseSection* collapseSection();

private:
	QVBoxLayout *m_layout;
	MenuSectionWidget* m_section;
	MenuCollapseSection *m_collapse;

};

} // namespace scopy
#endif // MENUSECTIONWIDGET_H
