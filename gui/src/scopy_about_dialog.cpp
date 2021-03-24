#include "ui_scopy_about_dialog.h"

#include <iio.h>

#include <gnuradio/constants.h>

#include <QDateTime>
#include <QDebug>
#include <QFile>

#include <libm2k/contextbuilder.hpp>
#include <scopy/gui/scopy_about_dialog.hpp>
#include <utils.hpp>

using namespace scopy::gui;

ScopyAboutDialog::ScopyAboutDialog(QWidget* parent)
	: QDialog(parent)
	, m_ui(new Ui::ScopyAboutDialog)
	, m_landingPage(QUrl("qrc:/html-pages/about.html")) // this doesn't work yet
{
	m_ui->setupUi(this);
	m_ui->lblCenteredUnderLogo->setText("Released under GPLv3");

	// Is this neccessary? I could not find this stylesheet
	//	QString stylesheet = Util::loadStylesheetFromFile(":stylesheets/stylesheets/browserStylesheet.qss");
	//	m_ui->textBrowserAbout->document()->setDefaultStyleSheet(stylesheet);

	m_ui->textBrowserAbout->setWordWrapMode(QTextOption::WordWrap);
	m_ui->textBrowserAbout->setSource(m_landingPage);
	m_ui->lblCheckForUpdates->setText("");

	connect(m_ui->btnBack, SIGNAL(clicked()), m_ui->textBrowserAbout, SLOT(backward()));
	connect(m_ui->btnOk, SIGNAL(clicked()), this, SLOT(accept()));
	connect(this, SIGNAL(finished(int)), this, SLOT(dismiss(int)));
	connect(m_ui->btnCheckForUpdates, &QPushButton::clicked, [=](int state) {
		m_ui->lblCheckForUpdates->setText(tr("Checking server for updates ... "));
		Q_EMIT forceCheckForUpdates();
	});
}

void ScopyAboutDialog::dismiss(int)
{
	m_ui->textBrowserAbout->clearHistory();
	m_ui->textBrowserAbout->setSource(m_landingPage);
}

ScopyAboutDialog::~ScopyAboutDialog() { delete m_ui; }

void ScopyAboutDialog::updateCheckUpdateLabel(qint64 timestamp)
{
	if (timestamp) {
		m_ui->lblCheckForUpdates->setText(
			tr("Last checked for updates at ") +
			QDateTime::fromMSecsSinceEpoch(timestamp).toString("dddd d MMMM yyyy hh:mm:ss"));
	} else {
		m_ui->lblCheckForUpdates->setText(tr("Check for updates failed ... "));
	}
}
