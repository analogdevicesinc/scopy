#ifndef INSTRUMENTMENU_H
#define INSTRUMENTMENU_H

#include <compositewidget.h>
#include <QMap>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QButtonGroup>

namespace scopy {

class InstrumentMenu : public QWidget, public CompositeWidget
{
	Q_OBJECT
public:
	InstrumentMenu(QWidget *parent);
	~InstrumentMenu();

	void add(QWidget *w) override;
	void add(QString itemId, QWidget *w);
	void remove(QWidget *w) override;
	void colapseAll();

	QButtonGroup *btnGroup() const;
private:
	QMap<QString, QWidget *> m_widgetMap;
	int m_uuid;
	QScrollArea *m_scroll;
	QVBoxLayout *m_layScroll;
	QSpacerItem *m_spacer;
	QButtonGroup *m_btnGroup;

	QString widgetName(QWidget *w);
};

} // namespace scopy

#endif // INSTRUMENTMENU_H
