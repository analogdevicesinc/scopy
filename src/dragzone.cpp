#include "dragzone.h"
#include "ui_dragzone.h"

using namespace adiscope;

DragZone::DragZone(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DragZone)
{
	ui->setupUi(this);
	position = 8;
	setAcceptDrops(true);
	this->installEventFilter(this);
}

DragZone::~DragZone()
{
	delete ui;
	this->removeEventFilter(this);
}

void DragZone::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat("menu/option")){
		short from = (short)event->mimeData()->data("menu/option")[1];
		if (from == position){
			event->ignore();
			return;
		}
	}
	event->accept();
}

void DragZone::dragMoveEvent(QDragMoveEvent *event)
{
	Q_EMIT highlightLastSeparator(true);
	event->accept();
}

void DragZone::dragLeaveEvent(QDragLeaveEvent *event)
{
	Q_EMIT highlightLastSeparator(false);
	event->accept();
}

void DragZone::dropEvent(QDropEvent *event)
{
	Q_EMIT highlightLastSeparator(false);
	short from, to;
	if (event->source() == this && event->possibleActions() & Qt::MoveAction){
		return;
	}
	if (event->mimeData()->hasFormat("menu/option")){
		from = (short)event->mimeData()->data("menu/option")[1];
		to = (short)position;
		Q_EMIT requestPositionChange(from, to, true);
	}
}

int DragZone::getPosition() const
{
	return position;
}

void DragZone::setPosition(int value)
{
	position = value;
}

bool DragZone::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::DragEnter){
		QDragEnterEvent *enterEvent = static_cast<QDragEnterEvent *>(event);
		if (!enterEvent->mimeData()->hasFormat("menu/option"))
			return true;
		}
	return QWidget::event(event);
}
