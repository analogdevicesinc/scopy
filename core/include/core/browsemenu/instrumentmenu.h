#ifndef INSTRUMENTMENU_H
#define INSTRUMENTMENU_H

#include <compositewidget.h>
#include <QMap>
#include <QScrollArea>
#include <QVBoxLayout>

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

private:
	QMap<QString, QWidget *> m_widgetMap;
	int m_uuid;
	QScrollArea *m_scroll;
	QVBoxLayout *m_layScroll;
	QSpacerItem *m_spacer;

	QString widgetName(QWidget *w);
};

} // namespace scopy

#endif // INSTRUMENTMENU_H
