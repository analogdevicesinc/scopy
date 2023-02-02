#include "scopyhomepage.h"
#include "ui_scopyhomepage.h"

ScopyHomePage::ScopyHomePage(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ScopyHomePage)
{
    ui->setupUi(this);    

}

ScopyHomePage::~ScopyHomePage()
{
    delete ui;
}
