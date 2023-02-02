#include "scopymainwindow.h"
#include "ui_scopymainwindow.h"
#include "scopyhomepage.h"
#include <QLabel>

ScopyMainWindow::ScopyMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ScopyMainWindow)
{
    ui->setupUi(this);
    auto tb = ui->wToolBrowser;
    auto ts = ui->wsToolStack;
    ts->addTool("home", new ScopyHomePage());
    ts->detachTool("home");

    ts->addTool("home1", new QLabel("home1"));
    ts->addTool("home2", new QLabel("home2"));
    ts->addTool("home3", new QLabel("home3"));

    connect(tb,&ToolBrowser::toolSelected,ts, &ToolStack::showTool);
//    connect(tb,&ToolBrowser::detach,ts, &ToolStack::showTool);
    ts->attachTool("home");


}

ScopyMainWindow::~ScopyMainWindow()
{
    delete ui;
}

