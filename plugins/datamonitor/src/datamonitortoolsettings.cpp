#include "datamonitortoolsettings.hpp"

#include <QFileDialog>
#include <datamonitorutils.hpp>
#include <menucollapsesection.h>
#include <menuheader.h>
#include <menusectionwidget.h>
#include <qboxlayout.h>
#include <spinbox_a.hpp>
#include <stylehelper.h>

#include <datamonitorstylehelper.hpp>

using namespace scopy;
using namespace datamonitor;

DataMonitorToolSettings::DataMonitorToolSettings(QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);

	MenuHeaderWidget *header =
		new MenuHeaderWidget(QString("Settings"), QPen(StyleHelper::getColor("ScopyBlue")), this);
	mainLayout->addWidget(header);

	MenuSectionWidget *readIntervalContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *readIntervalSection =
		new MenuCollapseSection("Read interval", MenuCollapseSection::MHCW_NONE, readIntervalContainer);

	readIntervalSection->contentLayout()->setSpacing(10);

	PositionSpinButton *readInterval = new PositionSpinButton(
		{
			{"s", 1},
			{"min", 60},
		},
		"", 0, (double)((long)1 << 31), false, false, readIntervalSection);

	readInterval->setValue(DataMonitorUtils::getReadIntervalDefaul());

	readIntervalSection->contentLayout()->addWidget(readInterval);
	readIntervalContainer->contentLayout()->addWidget(readIntervalSection);
	mainLayout->addWidget(readIntervalContainer);

	MenuSectionWidget *logDataContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *logDataSection =
		new MenuCollapseSection("Log data", MenuCollapseSection::MHCW_NONE, logDataContainer);
	logDataSection->contentLayout()->setSpacing(10);

	dataLoggingFilePath = new QLineEdit(logDataSection);
	dataLoggingFilePath->setReadOnly(true);

	connect(dataLoggingFilePath, &QLineEdit::textChanged, this, [=, this](QString path) {
		if(filename.isEmpty() && dataLoggingFilePath->isEnabled()) {
			dataLoggingFilePath->setText(tr("No file selected"));
			dataLoggingFilePath->setStyleSheet("color:red");
		} else {
			dataLoggingFilePath->setStyleSheet("color:white");
			Q_EMIT pathChanged(path);
		}
	});

	dataLoggingBrowseBtn = new QPushButton("Browse", logDataSection);
	connect(dataLoggingBrowseBtn, &QPushButton::clicked, this, &DataMonitorToolSettings::chooseFile);

	dataLoggingBtn = new QPushButton("Log data", logDataSection);
	connect(dataLoggingBtn, &QPushButton::clicked, this,
		[=, this]() { Q_EMIT requestDataLogging(dataLoggingFilePath->text()); });

	logDataSection->contentLayout()->addWidget(new QLabel("Choose file"));
	logDataSection->contentLayout()->addWidget(dataLoggingFilePath);
	logDataSection->contentLayout()->addWidget(dataLoggingBrowseBtn);
	logDataSection->contentLayout()->addWidget(dataLoggingBtn);

	logDataContainer->contentLayout()->addWidget(logDataSection);
	mainLayout->addWidget(logDataContainer);

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	mainLayout->addItem(spacer);

	connect(readInterval, &PositionSpinButton::valueChanged, this, &DataMonitorToolSettings::readIntervalChanged);

	DataMonitorStyleHelper::DataMonitorToolSettingsStyle(this);
}

void DataMonitorToolSettings::chooseFile()
{
	QString selectedFilter;
	filename = QFileDialog::getSaveFileName(
		this, tr("Export"), "", tr("Comma-separated values files (*.csv);;All Files(*)"), &selectedFilter,
		QFileDialog::Options(QFileDialog::DontUseNativeDialog));
	dataLoggingFilePath->setText(filename);
}
