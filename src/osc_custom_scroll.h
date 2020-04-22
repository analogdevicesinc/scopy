#ifndef OSCCUSTOMSCROLL_H
#define OSCCUSTOMSCROLL_H

#include <QEvent>
#include <QScrollArea>
#include <QScroller>
#include <QTimer>

namespace adiscope {
class OscCustomScrollArea : public QScrollArea
{
	Q_OBJECT
public:
	OscCustomScrollArea(QWidget* parent = 0);

public Q_SLOTS:
	void enterEvent(QEvent*);
	void leaveEvent(QEvent*);

private:
	QScroller* scroll;
	bool inside;
	// QScrollBar *bar;
	bool disableCursor;
};
} // namespace adiscope

#endif // OSCCUSTOMSCROLL_H
