#include "contextbrowseritem.h"
#include "ui_contextbrowseritem.h"

ContextBrowserItem::ContextBrowserItem(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ContextBrowserItem)
{
    ui->setupUi(this);
}

ContextBrowserItem::~ContextBrowserItem()
{
    delete ui;
}
