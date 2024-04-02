#include "dataloggingmenu.hpp"

#include <QFileDialog>
#include <datamonitorstylehelper.hpp>
#include <menucollapsesection.h>
#include <menusectionwidget.h>

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
		new MenuCollapseSection("Data logging", MenuCollapseSection::MHCW_NONE, logDataContainer);
	logDataSection->contentLayout()->setSpacing(10);

	dataLoggingFilePath = new ProgressLineEdit(logDataSection);
	dataLoggingFilePath->getLineEdit()->setReadOnly(true);

	connect(dataLoggingFilePath->getLineEdit(), &QLineEdit::textChanged, this, [=, this](QString path) {
		if(filename.isEmpty() && dataLoggingFilePath->getLineEdit()->isEnabled()) {
			dataLoggingFilePath->getLineEdit()->setText(tr("No file selected"));
			dataLoggingFilePath->getLineEdit()->setStyleSheet("color:red");
		} else {
			dataLoggingFilePath->getLineEdit()->setStyleSheet("color:white");
			Q_EMIT pathChanged(path);
		}
	});

	dataLoggingBrowseBtn = new QPushButton("Browse", logDataSection);
	connect(dataLoggingBrowseBtn, &QPushButton::clicked, this, &DataLoggingMenu::chooseFile);

	dataLoggingBtn = new QPushButton("Save data", logDataSection);
	connect(dataLoggingBtn, &QPushButton::clicked, this, [=, this]() {
		updateDataLoggingStatus(ProgressBarState::BUSSY);
		Q_EMIT requestDataLogging(dataLoggingFilePath->getLineEdit()->text());
	});

	dataLoadingBtn = new QPushButton("Load data", logDataSection);
	connect(dataLoadingBtn, &QPushButton::clicked, this, [=, this]() {
		updateDataLoggingStatus(ProgressBarState::BUSSY);
		Q_EMIT requestDataLoading(dataLoggingFilePath->getLineEdit()->text());
	});

	logDataSection->contentLayout()->addWidget(new QLabel("Choose file"));
	logDataSection->contentLayout()->addWidget(dataLoggingFilePath);
	logDataSection->contentLayout()->addWidget(dataLoggingBrowseBtn);
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
	if(status == ProgressBarState::BUSSY) {
		dataLoggingFilePath->getProgressBar()->startProgress();
		dataLoggingFilePath->getProgressBar()->setBarColor(StyleHelper::getColor("ProgressBarBusy"));
	}
}

void DataLoggingMenu::chooseFile()
{
	QString selectedFilter;
	filename = QFileDialog::getSaveFileName(
		this, tr("Export"), "", tr("Comma-separated values files (*.csv);;All Files(*)"), &selectedFilter,
		QFileDialog::Options(QFileDialog::DontUseNativeDialog));
	dataLoggingFilePath->getLineEdit()->setText(filename);
}
