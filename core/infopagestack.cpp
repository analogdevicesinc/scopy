#include "infopagestack.h"
#include "gui/customanimation.h"
#include <QParallelAnimationGroup>
#include <QLoggingCategory>
#include <QDebug>

using namespace adiscope;

Q_LOGGING_CATEGORY(CAT_INFOPAGESTACK, "InfoPageStack")

InfoPageStack::InfoPageStack(QWidget *parent)
	: MapStackedWidget(parent)
{
	hc = new HomepageControls(this);
	this->installEventFilter(hc);

	speed = 200;
	animationType = QEasingCurve::InOutCubic;
	active = false;
	now = QPoint(0, 0);
	current = 0;
	next = 0;
	hc->setVisible(true);
	hc->raise();
	qDebug(CAT_INFOPAGESTACK)<<"ctor";
}

InfoPageStack::~InfoPageStack()
{
	this->removeEventFilter(hc);
	qDebug(CAT_INFOPAGESTACK)<<"dtor";
}

void InfoPageStack::add(QString key, QWidget *w)
{
	MapStackedWidget::add(key,w);
	if(count() == 1) {
		hc->raise();
	}
	hc->setVisible(count() > 1);

}

bool InfoPageStack::show(QString key)
{
	auto ret = MapStackedWidget::show(key);
	hc->raise();
	hc->setVisible(count() > 1);
	return ret;
}

bool InfoPageStack::slideInKey(QString key, int direction) {
	QWidget *w = map.value(key);
	if(!w)
		return false;
	slideInWidget(w, direction);
	return true;
}

void InfoPageStack::animationDone()
{
	setCurrentIndex(next);
//	widget(current)->hide();
//	widget(current)->move(now);
	active = false;
	hc->raise();
}

void InfoPageStack::slideInWidget(QWidget *newWidget, int direction)
{
	if (active) {
		if (this->next != indexOf(newWidget)) {
			animationDone();
		} else {
			return;
		}
	}
	active = true;

	int current = currentIndex();
	int next = indexOf(newWidget);
	if (current == next) {
		active = false;
		return;
	}

	int offsetx = frameRect().width();
	int offsety = frameRect().height();
	widget(next)->setGeometry(0, 0, offsetx, offsety);

	if (direction < 0) {
		offsetx = - offsetx;
		offsety = 0;
	} else {
		offsety = 0;
	}

	QPoint pnext = widget(next)->pos();
	QPoint pcurrent = widget(current)->pos();
	now = pcurrent;

	widget(next)->move(pnext.x() - offsetx, pnext.y() - offsety);
	widget(next)->show();
	widget(next)->raise();

	CustomAnimation *animNow = new CustomAnimation(widget(current), "pos");
	animNow->setDuration(speed);
	animNow->setEasingCurve(animationType);
	animNow->setStartValue(QPoint(pcurrent.x(), pcurrent.y()));
	animNow->setEndValue(QPoint(offsetx + pcurrent.x(), offsety + pcurrent.y()));
	CustomAnimation *animNext = new CustomAnimation(widget(next), "pos");
	animNext->setDuration(speed);
	animNext->setEasingCurve(animationType);
	animNext->setStartValue(QPoint(-offsetx + pnext.x(), offsety + pnext.y()));
	animNext->setEndValue(QPoint(pnext.x(), pnext.y()));
	QParallelAnimationGroup *animGroup = new QParallelAnimationGroup(this);

	animGroup->addAnimation(animNow);
	animGroup->addAnimation(animNext);

	connect(animGroup, &QParallelAnimationGroup::finished, this, &InfoPageStack::animationDone);

	this->next = next;
	this->current = current;
	active = true;
	animGroup->start();
}

HomepageControls *InfoPageStack::getHomepageControls() const
{
	return hc;
}
