#include "menuoption.h"
#include "ui_menuoption.h"

#include "customPushButton.hpp"
#include <QIcon>
#include <QSize>
#include <iostream>
#include <QDebug>
#include <QMimeData>
#include <QMimeType>
#include <QDrag>
#include <QBitmap>
#include <iostream>
#include <QPaintEvent>
#include <QPainter>
#include "dynamicWidget.hpp"
using namespace adiscope;


MenuOption::MenuOption(QString toolName, QString iconPath,
		       int position, bool usesCustomBtn,
		       QWidget *parent) :
	QWidget(parent),
	ui(new Ui::MenuOption),
	toolName(toolName),
	iconPath(iconPath),
	position(position),
	usesCustomBtn(usesCustomBtn),
	botSep(nullptr),
	topSep(nullptr),
	detached(false)
{
	ui->setupUi(this);
	ui->toolBtn->setText(toolName);
	ui->toolBtn->setIcon(QIcon(iconPath));

	setAcceptDrops(true);
	setDynamicProperty(this, "allowHover", true);

	if (usesCustomBtn){
		QWidget *button = ui->horizontalLayout->itemAt(1)->widget();
		CustomPushButton *customButton = new CustomPushButton(this);
		customButton->setGeometry(button->geometry());
		customButton->setSizePolicy(button->sizePolicy());
		customButton->setStyleSheet(button->styleSheet());
		customButton->setCheckable(true);
		ui->horizontalLayout->removeWidget(button);
		delete button;
		ui->horizontalLayout->insertWidget(1, customButton);
	}
	topSep = new QFrame(this);
	topSep->setStyleSheet("background-color: transparent");
	topSep->setFrameShadow(QFrame::Plain);
	topSep->setLineWidth(1);
	topSep->setFrameShape(QFrame::HLine);
	topSep->setStyleSheet("color: rgba(255,255,255,50);");
	ui->verticalLayout->insertWidget(0, topSep);
	QSizePolicy sp_retainTopSep = topSep->sizePolicy();
	sp_retainTopSep.setRetainSizeWhenHidden(true);
	topSep->setSizePolicy(sp_retainTopSep);
	topSep->setVisible(false);

	if (position == 8){
		botSep = new QFrame(this);
		botSep->setFrameShadow(QFrame::Plain);
		botSep->setLineWidth(1);
		botSep->setFrameShape(QFrame::HLine);
		botSep->setStyleSheet("color: rgba(255,255,255,50);");
		ui->verticalLayout->insertWidget(2, botSep);
		QSizePolicy sp_retainBotSep = botSep->sizePolicy();
		sp_retainBotSep.setRetainSizeWhenHidden(true);
		botSep->setSizePolicy(sp_retainBotSep);
		botSep->setVisible(false);
	}

	this->installEventFilter(this);
}

MenuOption::~MenuOption()
{
	this->removeEventFilter(this);
	delete ui;
}

QPushButton *MenuOption::getToolBtn()
{
	return ui->toolBtn;
}
QPushButton *MenuOption::getToolStopBtn()
{
	if (usesCustomBtn)
		return static_cast<QPushButton *>(ui->horizontalLayout->itemAt(1)->widget());
	return ui->toolStopButton;
}

void MenuOption::setPosition(int position)
{
	this->position = position;
	if (position != 8){
		if (botSep != nullptr){
			ui->verticalLayout->removeWidget(botSep);
			delete botSep;
			botSep = nullptr;
		}
	} else {
		if (botSep == nullptr){
			botSep = new QFrame(this);
			botSep->setFrameShadow(QFrame::Plain);
			botSep->setLineWidth(1);
			botSep->setFrameShape(QFrame::HLine);
			botSep->setStyleSheet("color: rgba(255,255,255,50);");
			ui->verticalLayout->insertWidget(2, botSep);
			QSizePolicy sp_retainBotSep = botSep->sizePolicy();
			sp_retainBotSep.setRetainSizeWhenHidden(true);
			botSep->setSizePolicy(sp_retainBotSep);
			botSep->setVisible(false);
		}
	}
}

int MenuOption::getPosition()
{
	return position;
}

QString MenuOption::getName()
{
	return toolName;
}

void MenuOption::setDetached(bool detached)
{
	this->detached = detached;
}

bool MenuOption::isDetached()
{
	return detached;
}

void MenuOption::highlightBotSeparator(bool on)
{
	botSep->setVisible(on);
}

void MenuOption::highlightNeighbour(bool on)
{
	if (on)
		highlightTopSeparator();
	else
		disableSeparatorsHighlight();
}

void MenuOption::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton){
		dragStartPosition = event->pos();
	}
}

void MenuOption::mouseMoveEvent(QMouseEvent *event)
{
	if (!(event->buttons() & Qt::LeftButton)){
		return;
	}

	if ((event->pos() - dragStartPosition).manhattanLength() <
			QApplication::startDragDistance()){
		return;
	}
	QDrag *drag = new QDrag(this);
	QMimeData *mimeData = new QMimeData;

	QByteArray itemData;
	QDataStream dataStream(&itemData, QIODevice::WriteOnly);
	dataStream << (short)position;
	mimeData->setData("menu/option", itemData);

	QPixmap pix;
	pix = this->grab().scaled(this->geometry().width(),
				  this->geometry().height());

	QSizePolicy sp_retainSize = this->sizePolicy();
	sp_retainSize.setRetainSizeWhenHidden(true);
	this->setSizePolicy(sp_retainSize);
	this->setVisible(false);

	drag->setPixmap(pix);

	drag->setMimeData(mimeData);
	Q_EMIT enableInfoWidget(true);
	Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
	Q_EMIT enableInfoWidget(false);
	this->setVisible(true);

}

void MenuOption::dragEnterEvent(QDragEnterEvent *event)
{
	Q_EMIT changeText("Move");
	auto w = this->geometry().width();
	auto h = this->geometry().height();
	topDragbox.setRect(0, 0, w, h/2);
	botDragbox.setRect(0, h/2, w, h/2);

	if (event->mimeData()->hasFormat("menu/option")){
		short from = (short)event->mimeData()->data("menu/option")[1];
		if (from == position){
			event->ignore();
			return;
		}
	}
	event->accept();
}

void MenuOption::dragMoveEvent(QDragMoveEvent *event)
{
	setDynamicProperty(this, "allowHover", false);
	if (event->answerRect().intersects(topDragbox)){
		disableSeparatorsHighlight();
		highlightTopSeparator();
		event->accept();
	} else if (event->answerRect().intersects(botDragbox) &&
				this->position == 8){
			disableSeparatorsHighlight();
			highlightBotSeparator();
			event->accept();
	} else if (event->answerRect().intersects(botDragbox)){
			disableSeparatorsHighlight();
			Q_EMIT highlight(true, position + 1);
			event->accept();
	} else {
		disableSeparatorsHighlight();
		event->ignore();
	}
}

void MenuOption::dragLeaveEvent(QDragLeaveEvent *event)
{
	setDynamicProperty(this, "allowHover", true);
	disableSeparatorsHighlight();
	event->accept();
}

void MenuOption::dropEvent(QDropEvent *event)
{
	disableSeparatorsHighlight();
	short from, to;
	if (event->source() == this && event->possibleActions() & Qt::MoveAction){
		return;
	}
	bool dropAfter = (botDragbox.contains(event->pos()) && position == 8);
	bool dropAfterOnly = botDragbox.contains(event->pos());
	if (event->mimeData()->hasFormat("menu/option")){
		from = (short)event->mimeData()->data("menu/option")[1];
		to = (short)position;
		if (!dropAfter && dropAfterOnly) to++;
		Q_EMIT requestPositionChange(from, to, dropAfter);
	}
}

void MenuOption::enterEvent(QEvent *event)
{
	setDynamicProperty(this, "allowHover", true);
	event->accept();
}

void MenuOption::leaveEvent(QEvent *event)
{
	setDynamicProperty(this, "allowHover", false);
	event->accept();
}

void MenuOption::paintEvent(QPaintEvent *pe)
{
	QStyleOption o;
	o.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(
				QStyle::PE_Widget, &o, &p, this);
}

void MenuOption::disableSeparatorsHighlight(){
	topSep->setVisible(false);
	if (botSep != nullptr)
		botSep->setVisible(false);
	Q_EMIT highlight(false, position + 1);
}

void MenuOption::highlightTopSeparator(){
	topSep->setVisible(true);
}

void MenuOption::highlightBotSeparator()
{
	botSep->setVisible(true);
}

bool MenuOption::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::DragEnter){
		QDragEnterEvent *enterEvent = static_cast<QDragEnterEvent *>(event);
		if (!enterEvent->mimeData()->hasFormat("menu/option"))
			return true;
		}
	return QWidget::event(event);
}
