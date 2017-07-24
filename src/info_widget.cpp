#include "info_widget.h"
#include "ui_info_widget.h"
#include <QTimer>

using namespace adiscope;

InfoWidget::InfoWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::InfoWidget)
{
	ui->setupUi(this);
	timer =  new QTimer();
	connect(timer, SIGNAL(timeout()), this,
		SLOT(updatePosition()));
	this->hide();
}

InfoWidget::~InfoWidget()
{
	delete ui;
}

void InfoWidget::updatePosition()
{
	QPoint p = this->parentWidget()->mapFromGlobal(QCursor::pos());
	//Position the widget in the bottom right corner of the pixmap
	//that is dragged
	this->move(p.x() + 135, p.y() + 68);
}

void InfoWidget::setText(QString text)
{
	ui->label->setText(text);
}

void InfoWidget::enable(bool on)
{
	if (on){
		timer->start(1);
		this->show();
	} else {
		timer->stop();
		this->hide();
	}
}


