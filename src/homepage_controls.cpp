#include "homepage_controls.h"
#include "ui_homepage_controls.h"
#include <QDebug>

using namespace adiscope;

HomepageControls::HomepageControls(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::HomepageControls)
{
	ui->setupUi(this);

	connect(ui->leftBtn, &QPushButton::clicked, [=](){
		Q_EMIT goLeft();
	});
	connect(ui->rightBtn, &QPushButton::clicked, [=](){
		Q_EMIT goRight();
	});
	connect(ui->openBtn, &QPushButton::clicked, [=](){
		Q_EMIT openFile();
	});

	ui->openBtn->hide();

	updatePosition();
}

HomepageControls::~HomepageControls()
{
	delete ui;
}

void HomepageControls::updatePosition()
{
	move(parentWidget()->width() - 150, geometry().topLeft().y());
}

bool HomepageControls::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::Resize) {
		updatePosition();
		return false;
	}

	return QObject::eventFilter(watched, event);
}
