#ifndef INFOPAGESTACK_H
#define INFOPAGESTACK_H

#include "mapstackedwidget.h"
#include <QEasingCurve>
#include "gui/homepage_controls.h"

namespace adiscope {

// implement slide to-from  next/prev page - emits to button selection group

class InfoPageStack : public MapStackedWidget
{
	Q_OBJECT
public:


	explicit InfoPageStack(QWidget *parent = nullptr);
	~InfoPageStack();
	void add(QString key, QWidget *w) override;

	HomepageControls *getHomepageControls() const;

public Q_SLOTS:
	bool show(QString key) override;
	bool slideInKey(QString key, bool left2right = false);

protected Q_SLOTS:
	void animationDone();

private:
	enum direction {
		LEFT2RIGHT,
		RIGHT2LEFT
	};
	void slideInWidget(QWidget *newWidget, InfoPageStack::direction direction);

	HomepageControls *hc;

	enum QEasingCurve::Type animationType;
	int speed;

	bool active;
	QPoint now;
	int current;
	int next;



};
}
#endif // INFOPAGESTACK_H
