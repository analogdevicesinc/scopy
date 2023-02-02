#include "addcontextordemoinfopage.h"
#include "ui_addcontextordemoinfopage.h"

AddContextOrDemoInfoPage::AddContextOrDemoInfoPage(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::AddContextOrDemoInfoPage)
{
	ui->setupUi(this);
}

AddContextOrDemoInfoPage::~AddContextOrDemoInfoPage()
{
	delete ui;
}
