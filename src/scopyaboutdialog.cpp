#include "scopyaboutdialog.hpp"
#include "ui_scopyaboutdialog.h"
#include <libm2k/contextbuilder.hpp>
#include <iio.h>
#include <gnuradio/constants.h>
#include <QFile>
#include <QDebug>

using namespace adiscope;


ScopyAboutDialog::ScopyAboutDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ScopyAboutDialog),
	landingPage(QUrl("qrc:/about.html"))
{
	ui->setupUi(this);
	ui->centeredUnderLogo->setText("Released under GPLv3");

	ui->aboutTextBrowser->setWordWrapMode(QTextOption::WordWrap);
	ui->aboutTextBrowser->setSource(landingPage);
	connect(ui->backButton,SIGNAL(clicked()),ui->aboutTextBrowser,SLOT(backward()));
	connect(ui->okButton,SIGNAL(clicked()),this,SLOT(accept()));
	connect(this,SIGNAL(finished(int)),this,SLOT(dismiss(int)));
}

void ScopyAboutDialog::dismiss(int)
{
	ui->aboutTextBrowser->clearHistory();
	ui->aboutTextBrowser->setSource(landingPage);
}

ScopyAboutDialog::~ScopyAboutDialog()
{
	delete ui;
}
