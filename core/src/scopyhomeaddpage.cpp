#include "scopyhomeaddpage.h"
#include "ui_scopyhomeaddpage.h"
#include "pluginbase/contextprovider.h"

ScopyHomeAddPage::ScopyHomeAddPage(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ScopyHomeAddPage)
{
	ui->setupUi(this);
	connect(ui->btnVerify,SIGNAL(clicked()),this,SLOT(verify()));
	connect(ui->btnAdd,SIGNAL(clicked()),this,SLOT(add()));
	ui->btnAdd->setVisible(false);
}

void ScopyHomeAddPage::add() {
	QString uri = ui->editUri->text();
	Q_EMIT requestAddDevice(uri);
	Q_EMIT requestDevice(uri);
}

void ScopyHomeAddPage::verify() {
	QString uri = ui->editUri->text();
	struct iio_context* ctx = ContextProvider::GetInstance()->open(uri);
	if(ctx) {
		ui->ctxInfoBrowser->setText(QString::fromLocal8Bit(iio_context_get_name(ctx)));
		ContextProvider::GetInstance()->close(uri);
		ui->btnVerify->setVisible(false);
		ui->btnAdd->setVisible(true);
	}

}

ScopyHomeAddPage::~ScopyHomeAddPage()
{
	delete ui;
}
