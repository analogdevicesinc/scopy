#include "scopymainwindow.h"
#include "ui_scopymainwindow.h"
#include "scopyhomepage.h"
#include <QLabel>
#include "logging_categories.h"
#include <contextscanner.h>
#include <contextmanager.h>

using namespace adiscope;
ScopyMainWindow::ScopyMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ScopyMainWindow)
{

	auto cm = ContextManager::GetInstance();
	ContextScanner *cs = new ContextScanner(this);
	cs->startScan(5000);
	cm->open("ip:192.168.2.1");
	cm->open("ip:192.168.2.1");
	cm->open("ip:192.168.2.1");
	cm->close("ip:192.168.2.1");
	auto cd = ContextManager::GetInstance();
	cd->close("ip:192.168.2.1");
	cd->close("ip:192.168.2.1");




}

ScopyMainWindow::~ScopyMainWindow()
{
    delete ui;
}

