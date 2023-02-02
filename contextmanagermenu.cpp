#include "contextmanagermenu.h"
#include "ui_contextmanagermenu.h"

ContextManagerMenu::ContextManagerMenu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContextManagerMenu)
{
    ui->setupUi(this);
}

ContextManagerMenu::~ContextManagerMenu()
{
    delete ui;
}
