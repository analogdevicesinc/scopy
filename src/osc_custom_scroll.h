#ifndef OSCCUSTOMSCROLL_H
#define OSCCUSTOMSCROLL_H

#include <QScrollArea>
#include <QEvent>
#include <QTimer>
#include <QScroller>

namespace adiscope {
class OscCustomScrollArea : public QScrollArea
{
	Q_OBJECT
public:
	OscCustomScrollArea(QWidget *parent = 0);

public Q_SLOTS:
	void enterEvent(QEvent *);
	void leaveEvent(QEvent *);

private:
	QScroller *scroll;
	bool inside;
	//QScrollBar *bar;
	bool disableCursor;
};
}

#endif // OSCCUSTOMSCROLL_H
