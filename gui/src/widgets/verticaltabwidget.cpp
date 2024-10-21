#include <verticaltabbar.h>
#include <verticaltabwidget.h>

using namespace scopy;

VerticalTabWidget::VerticalTabWidget(QWidget *parent)
	: QTabWidget(parent)
{
	setTabBar(new VerticalTabBar(this));
}
