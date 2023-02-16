#include "scopyhomeinfopage.h"
#include "ui_scopyhomeinfopage.h"

ScopyHomeInfoPage::ScopyHomeInfoPage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScopyHomeInfoPage)
{
    ui->setupUi(this);
}

ScopyHomeInfoPage::~ScopyHomeInfoPage()
{
    delete ui;
}
