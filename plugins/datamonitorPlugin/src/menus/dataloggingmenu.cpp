#include "dataloggingmenu.hpp"

#include <QFileDialog>
#include <datamonitorstylehelper.hpp>
#include <menucollapsesection.h>
#include <menusectionwidget.h>
#include <timemanager.hpp>

using namespace scopy;
using namespace datamonitor;

DataLoggingMenu::DataLoggingMenu(QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);

	MenuSectionWidget *logDataContainer = new MenuSectionWidget(this);
	MenuCollapseSection *logDataSection =
		new MenuCollapseSection("DATA LOGGING", MenuCollapseSection::MHCW_NONE, logDataContainer);
	logDataSection->contentLayout()->setSpacing(10);

	dataLoggingFilePath = new ProgressLineEdit(logDataSection);
	dataLoggingFilePath->getLineEdit()->setReadOnly(true);

	dataLoggingBrowseBtn = new QPushButton("Browse", logDataSection);
	connect(dataLoggingBrowseBtn, &QPushButton::clicked, this, &DataLoggingMenu::chooseFile);

	liveDataLoggingButton = new MenuOnOffSwitch("Live data logging", logDataSection);

	dataLoggingBtn = new QPushButton("Save data", logDataSection);

	dataLoadingBtn = new QPushButton("Import data", logDataSection);

	toggleButtonsEnabled(false);

	///// time manager timeout used for requesting continuous data logging
	auto &&timeTracker = TimeManager::GetInstance();
	connect(timeTracker, &TimeManager::timeout, this, [=, this]() {
		if(liveDataLoggingButton->onOffswitch()->isChecked()) {
			Q_EMIT requestLiveDataLogging(dataLoggingFilePath->getLineEdit()->text());
		}
	});

	connect(dataLoggingBtn, &QPushButton::clicked, this, [=, this]() {
		updateDataLoggingStatus(ProgressBarState::BUSY);
		Q_EMIT requestDataLogging(dataLoggingFilePath->getLineEdit()->text());
	});

	connect(liveDataLoggingButton->onOffswitch(), &QAbstractButton::toggled, this, [=, this](bool toggled) {
		m_liveDataLogging = toggled;
		dataLoggingBtn->setEnabled(!toggled);
		dataLoadingBtn->setEnabled(!toggled);
	});

	connect(dataLoadingBtn, &QPushButton::clicked, this, [=, this]() {
		updateDataLoggingStatus(ProgressBarState::BUSY);
		Q_EMIT requestDataLoading(dataLoggingFilePath->getLineEdit()->text());
	});

	connect(dataLoggingFilePath->getLineEdit(), &QLineEdit::textChanged, this, [=, this](QString path) {
		if(filename.isEmpty() && dataLoggingFilePath->getLineEdit()->isEnabled()) {
			dataLoggingFilePath->getLineEdit()->setText(tr("No file selected"));
			dataLoggingFilePath->getLineEdit()->setStyleSheet("color:red");
			toggleButtonsEnabled(false);

		} else {
			dataLoggingFilePath->getLineEdit()->setStyleSheet("color:white");
			toggleButtonsEnabled(true);
			Q_EMIT pathChanged(path);
		}
	});

	logDataSection->contentLayout()->addWidget(new QLabel("Choose file"));
	logDataSection->contentLayout()->addWidget(dataLoggingFilePath);
	logDataSection->contentLayout()->addWidget(dataLoggingBrowseBtn);
	logDataSection->contentLayout()->addWidget(liveDataLoggingButton);
	logDataSection->contentLayout()->addWidget(dataLoggingBtn);
	logDataSection->contentLayout()->addWidget(dataLoadingBtn);

	logDataContainer->contentLayout()->addWidget(logDataSection);

	mainLayout->addWidget(logDataContainer);

	DataMonitorStyleHelper::DataLoggingMenuStyle(this);
}

void DataLoggingMenu::updateDataLoggingStatus(ProgressBarState status)
{
	if(status == ProgressBarState::SUCCESS) {
		dataLoggingFilePath->getProgressBar()->setBarColor(StyleHelper::getColor("ProgressBarSuccess"));
	}
	if(status == ProgressBarState::ERROR) {
		dataLoggingFilePath->getProgressBar()->setBarColor(StyleHelper::getColor("ProgressBarError"));
	}
	if(status == ProgressBarState::BUSY) {
		dataLoggingFilePath->getProgressBar()->startProgress();
		dataLoggingFilePath->getProgressBar()->setBarColor(StyleHelper::getColor("ProgressBarBusy"));
	}
}

bool DataLoggingMenu::liveDataLogging() const { return m_liveDataLogging; }

void DataLoggingMenu::chooseFile()
{
	// turn off live data logging when switching files
	liveDataLoggingButton->onOffswitch()->setChecked(false);

	QString selectedFilter;
	filename = QFileDialog::getSaveFileName(
		this, tr("Export"), "", tr("Comma-separated values files (*.csv);;All Files(*)"), &selectedFilter,
		QFileDialog::Options(QFileDialog::DontUseNativeDialog));
	dataLoggingFilePath->getLineEdit()->setText(filename);
}

void DataLoggingMenu::toggleButtonsEnabled(bool en)
{
	dataLoggingBtn->setEnabled(en);
	dataLoadingBtn->setEnabled(en);
	liveDataLoggingButton->setEnabled(en);
}
