#include "detachdragzone.h"
#include "tool_launcher.hpp"
#include <QMimeData>

using namespace adiscope;

DetachDragZone::DetachDragZone(QWidget *parent) :
	ColoredQWidget(parent)
{
	ToolLauncher *tl = static_cast<ToolLauncher*>(parent->parent());
	if (tl->infoWidget != nullptr){
		connect(this, SIGNAL(changeText(QString)), tl->infoWidget,
			SLOT(setText(QString)));
		connect(this, SIGNAL(detachWidget(int)), tl,
			SLOT(detachToolOnPosition(int)));
	}

	setAcceptDrops(true);
	this->installEventFilter(this);
}

DetachDragZone::~DetachDragZone()
{
}

bool DetachDragZone::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::DragEnter){
		QDragEnterEvent *enterEvent = static_cast<QDragEnterEvent *>(event);
		if (!enterEvent->mimeData()->hasFormat("menu/option"))
			return true;
		}
	return QWidget::event(event);
}

void DetachDragZone::dragEnterEvent(QDragEnterEvent *event)
{

	Q_EMIT changeText(" Detach");
	event->accept();
}

void DetachDragZone::dragMoveEvent(QDragMoveEvent *event)
{
	event->accept();
}

void DetachDragZone::dragLeaveEvent(QDragLeaveEvent *event)
{
	Q_EMIT changeText(" Move");
	event->accept();
}

void DetachDragZone::dropEvent(QDropEvent *event)
{
	short position;
	if (event->source() == this && event->possibleActions() & Qt::MoveAction){
		return;
	}
	if (event->mimeData()->hasFormat("menu/option")){
		position = (short)event->mimeData()->data("menu/option")[1];
		Q_EMIT detachWidget(position);
	}
}
