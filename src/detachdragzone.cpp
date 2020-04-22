#include "detachdragzone.h"

#include "basemenuitem.h"
#include "tool_launcher.hpp"

#include <QMimeData>

using namespace adiscope;

DetachDragZone::DetachDragZone(QWidget* parent)
	: ColoredQWidget(parent)
{
	ToolLauncher* tl = static_cast<ToolLauncher*>(parent->parent());
	if (tl->infoWidget != nullptr) {
		connect(this, SIGNAL(changeText(QString)), tl->infoWidget, SLOT(setText(QString)));
	}

	setAcceptDrops(true);
	this->installEventFilter(this);
}

DetachDragZone::~DetachDragZone() {}

bool DetachDragZone::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::DragEnter) {
		QDragEnterEvent* enterEvent = static_cast<QDragEnterEvent*>(event);
		if (!enterEvent->mimeData()->hasFormat(BaseMenuItem::menuItemMimeDataType))
			return true;
	}
	return QWidget::event(event);
}

void DetachDragZone::dragEnterEvent(QDragEnterEvent* event)
{
	if (!event->source()) {
		event->ignore();
		return;
	}

	Q_EMIT changeText(" Detach");
	event->accept();
}

void DetachDragZone::dragMoveEvent(QDragMoveEvent* event) { event->accept(); }

void DetachDragZone::dragLeaveEvent(QDragLeaveEvent* event)
{
	Q_EMIT changeText(" Move");
	event->accept();
}

void DetachDragZone::dropEvent(QDropEvent* event)
{
	if (event->source() == this && event->possibleActions() & Qt::MoveAction) {
		return;
	}
	if (event->mimeData()->hasFormat(BaseMenuItem::menuItemMimeDataType)) {
		QObject* source = event->source();
		if (source) {
			static_cast<ToolMenuItem*>(source)->detach();
		}
	}
}
