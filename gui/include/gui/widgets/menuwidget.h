#ifndef MENUWIDGET_H
#define MENUWIDGET_H

#include <QWidget>
#include "compositewidget.h"
#include "menuheader.h"
#include "qscrollarea.h"
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QMap>
#include <scopy-gui_export.h>

namespace scopy {
namespace gui {
class SCOPY_GUI_EXPORT MenuWidget : public QWidget, public CompositeWidget
{
	Q_OBJECT
public:
	enum MenuAlignment
	{
		MA_TOPFIRST,
		MA_TOPLAST,
		MA_BOTTOMFIRST,
		MA_BOTTOMLAST
	};

	MenuWidget(QString name, QPen p, QWidget *parent);
	~MenuWidget();

	void add(QWidget *, QString name, MenuAlignment position);
	void add(QWidget *) override;
	void remove(QWidget *) override;
	void add(QWidget *, QString name);
	void remove(QString);
	MenuHeaderWidget *header();

	QWidget *findWidget(QString name);
	QString widgetName(QWidget *);

	void scrollTo(QWidget *);
	void collapseAll();
	void setCollapsed(QString name, bool b);

private:
	QMap<QString, QWidget *> m_widgetMap;
	QSpacerItem *m_spacer;
	QVBoxLayout *m_layScroll;
	MenuHeaderWidget *m_header;
	QScrollArea *scroll;
	int uuid;
};
} // namespace gui

} // namespace scopy

#endif // MENUWIDGET_H
