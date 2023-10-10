#include "verticalscrollarea.hpp"

#include <QEvent>

VerticalScrollArea::VerticalScrollArea(QWidget *parent)
	: QScrollArea(parent)
{
	setWidgetResizable(true);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
}

bool VerticalScrollArea::eventFilter(QObject *o, QEvent *e)
{
	if(o && o == widget() && e->type() == QEvent::Resize)
		setMinimumWidth(widget()->minimumSizeHint().width());
	return QScrollArea::eventFilter(o, e);
}
