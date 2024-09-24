#include "compositeheaderwidget.h"

using namespace scopy;

CompositeHeaderWidget::CompositeHeaderWidget(QString title, QWidget *parent)
	: BaseHeaderWidget(title, parent)
{}

CompositeHeaderWidget::~CompositeHeaderWidget() {}

void CompositeHeaderWidget::add(QWidget *w)
{
	int titleIndex = m_lay->indexOf(m_label);
	m_lay->insertWidget(titleIndex + 1, w);
}

void CompositeHeaderWidget::remove(QWidget *w) { m_lay->removeWidget(w); }
