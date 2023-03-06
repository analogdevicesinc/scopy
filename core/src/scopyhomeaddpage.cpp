#include "scopyhomeaddpage.h"
#include "ui_scopyhomeaddpage.h"
#include "iioutil/contextprovider.h"
#include <QtConcurrent>
#include <QFuture>

using namespace adiscope;
ScopyHomeAddPage::ScopyHomeAddPage(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::ScopyHomeAddPage)
{
	ui->setupUi(this);
	connect(ui->btnVerify,SIGNAL(clicked()),this,SLOT(futureverify()));


	fw = new QFutureWatcher<bool>(this);
	connect(fw,&QFutureWatcher<bool>::finished,this,[=](){
//		ui->ctxInfoBrowser->setText(QString::fromLocal8Bit(iio_context_get_name(ctx)));
		if(fw->result()==true) {
			ui->btnVerify->setVisible(false);
			ui->btnAdd->setVisible(true);
		}
	});

	connect(ui->btnAdd,SIGNAL(clicked()),this,SLOT(add()));
	ui->btnAdd->setVisible(false);
	pendingUri = "";
}


bool ScopyHomeAddPage::verify() {
	QString uri = ui->editUri->text();
	struct iio_context* ctx = ContextProvider::GetInstance()->open(uri);
	if(ctx) {		
		ContextProvider::GetInstance()->close(uri);
		return true;
	}	
	return false;

}

void ScopyHomeAddPage::add() {
	QString uri = ui->editUri->text();
	pendingUri = uri;
	Q_EMIT requestAddDevice("iio",uri);

}

void ScopyHomeAddPage::futureverify()
{
	QFuture<bool> f = QtConcurrent::run(this,&ScopyHomeAddPage::verify);
	fw->setFuture(f);
}

void ScopyHomeAddPage::deviceAddedToUi(QString id)
{
	if(!pendingUri.isEmpty()) {
		Q_EMIT requestDevice(id);
		pendingUri = "";
	}
}

ScopyHomeAddPage::~ScopyHomeAddPage()
{
	delete ui;
}
