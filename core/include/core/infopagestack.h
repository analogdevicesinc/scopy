#ifndef INFOPAGESTACK_H
#define INFOPAGESTACK_H

#include "device.h"
#include "mapstackedwidget.h"
#include <QEasingCurve>
#include "gui/homepage_controls.h"
#include "scopycore_export.h"

namespace scopy {

// implement slide to-from  next/prev page - emits to button selection group

class SCOPYCORE_EXPORT InfoPageStack : public MapStackedWidget
{
	Q_OBJECT
public:


	explicit InfoPageStack(QWidget *parent = nullptr);
	~InfoPageStack();
	void add(QString key, Device* d);

	HomepageControls *getHomepageControls() const;

	void add(QString key, QWidget* w) override;
	bool remove(QString key) override;
public Q_SLOTS:
	bool show(QString key) override;
	bool slideInKey(QString key, int direction);

protected Q_SLOTS:
	void animationDone();

private:
	QMap<QString, Device*> idDevMap;
	void slideInWidget(QWidget *newWidget, int direction);
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
