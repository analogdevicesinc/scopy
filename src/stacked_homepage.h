#ifndef STACKED_HOMEPAGE_H
#define STACKED_HOMEPAGE_H

#include <QWidget>
#include <QStackedWidget>
#include <QEasingCurve>

#include "homepage_controls.h"

namespace Ui {
class StackedHomepage;
}

namespace adiscope {
class StackedHomepage : public QStackedWidget
{
	Q_OBJECT

public:
	enum s_directions {
		LEFT2RIGHT,
		RIGHT2LEFT
	};

	explicit StackedHomepage(QWidget *parent = 0);
	~StackedHomepage();

	void addWidget(QWidget *widget);

public Q_SLOTS:
	void moveLeft();
	void moveRight();
	void openFile();

	void setSpeed(int speed);
	void setAnimation(enum QEasingCurve::Type animationType);
	void setWrap(bool wrap);

	void slideInNext();
	void slideInPrev();

protected Q_SLOTS:
	void animationDone();

Q_SIGNALS:
	void animationFinished();

private:
	void slideToIndex(int index);
	void slideInWidget(QWidget* newWidget, enum s_directions direction);

	HomepageControls *s_hc;
	enum QEasingCurve::Type s_animationType;
	int s_speed;
	bool s_wrap;
	bool s_active;
	QPoint s_now;
	int s_current;
	int s_next;
};
}
#endif // STACKED_HOMEPAGE_H
