#include "scopyaboutdialog.hpp"
#include "ui_scopyaboutdialog.h"
#include <libm2k/contextbuilder.hpp>
#include <iio.h>
#include <gnuradio/constants.h>
#include <QFile>
#include <utils.h>
#include <QDebug>
#include <QDateTime>

using namespace adiscope;


ScopyAboutDialog::ScopyAboutDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::ScopyAboutDialog),
	landingPage(QUrl("qrc:/about.html"))
{
	ui->setupUi(this);
	ui->centeredUnderLogo->setText("Released under GPLv3");

	QString stylesheet = Util::loadStylesheetFromFile(":stylesheets/stylesheets/browserStylesheet.qss");
	ui->aboutTextBrowser->document()->setDefaultStyleSheet(stylesheet);

	ui->aboutTextBrowser->setWordWrapMode(QTextOption::WordWrap);
	ui->aboutTextBrowser->setSource(landingPage);
	ui->checkForUpdatesLbl->setText("");

	connect(ui->backButton,SIGNAL(clicked()),ui->aboutTextBrowser,SLOT(backward()));
	connect(ui->okButton,SIGNAL(clicked()),this,SLOT(accept()));
	connect(this,SIGNAL(finished(int)),this,SLOT(dismiss(int)));
	connect(ui->checkForUpdatesBtn, &QPushButton::clicked, [=](int state) {
		Q_EMIT forceCheckForUpdates();
	});
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

void ScopyAboutDialog::updateCheckUpdateLabel(qint64 timestamp)
{
	if(timestamp)
		ui->checkForUpdatesLbl->setText("Last checked for updates at " + QDateTime::fromMSecsSinceEpoch(timestamp).toString("dddd d MMMM yyyy hh:mm:ss"));
	else
		ui->checkForUpdatesLbl->setText("Check updates failed");
}
