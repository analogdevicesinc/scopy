#include "dataloggerview.hpp"

using namespace adiscope;

DataLoggerView::DataLoggerView(bool lastValue, bool average, bool all, QWidget *parent) :
	QWidget(parent)
{
	dataLoggingWidget = new QWidget(this);
	dataLoggingLayout = new QVBoxLayout(dataLoggingWidget);

	dataLoggerFilter = new QComboBox(dataLoggingWidget);
	if (lastValue) {
		dataLoggerFilter->addItem(QString("Last Value"));
	}
	if (average) {
		dataLoggerFilter->addItem(QString("Average"));
	}
	if (all) {
		dataLoggerFilter->addItem(QString("All"));
	}
	dataLoggerFilter->setCurrentIndex(0);
	dataLoggerFilter->setDisabled(true);

	init();
}


void DataLoggerView::init()
{
	dataLoggingSwitch = new CustomSwitch(dataLoggingWidget);
	dataLoggingLayout->addWidget(dataLoggingSwitch);
	dataLoggingLayout->setAlignment(dataLoggingSwitch,Qt::AlignRight);

	dataLoggingLayout->addWidget(new QLabel("Choose file"));
	dataLoggingFilePath = new QLineEdit(dataLoggingWidget);
	dataLoggingFilePath->setReadOnly(true);
	dataLoggingFilePath->setDisabled(true);

	connect(dataLoggingFilePath, &QLineEdit::textChanged, this, [=](QString path){
		if (filename.isEmpty() && dataLoggingFilePath->isEnabled()) {
			dataLoggingFilePath->setText(tr("No file selected"));
			dataLoggingFilePath->setStyleSheet("color:red");
		} else {
			dataLoggingFilePath->setStyleSheet("color:white");
			Q_EMIT pathChanged(path);
		}
	});

	dataLoggingLayout->addWidget(dataLoggingFilePath);
	dataLoggingBrowseBtn = new QPushButton("Browse");
	dataLoggingBrowseBtn->setProperty("blue_button",true);
	dataLoggingBrowseBtn->setMinimumHeight(30);
	dataLoggingBrowseBtn->setDisabled(true);
	dataLoggingLayout->addWidget(dataLoggingBrowseBtn);

	connect(dataLoggingBrowseBtn, &QPushButton::clicked, this , & DataLoggerView::chooseFile);

	overwriteRadio = new QRadioButton("Overwrite");
	overwriteRadio->setChecked(true);
	overwriteRadio->setDisabled(true);
	dataLoggingLayout->addWidget(overwriteRadio);
	appendRadio = new QRadioButton("Append");
	dataLoggingLayout->addWidget(appendRadio);
	appendRadio->setDisabled(true);

	connect(overwriteRadio, &QRadioButton::toggled, [=](bool en) {
		appendRadio->setChecked(!en);
	});

	connect(appendRadio, &QRadioButton::toggled, [=](bool en) {
		overwriteRadio->setChecked(!en);
	});

	data_logging_timer = new PositionSpinButton({
													{"s", 1},
													{"min", 60},
													{"h", 3600}
												}, tr("Timer"), 0, 3600,
												true, false, this);

	data_logging_timer->setValue(5);
	data_logging_timer->setDisabled(true);

	connect(data_logging_timer, &PositionSpinButton::valueChanged, this, [=](){
		Q_EMIT timeIntervalChanged(data_logging_timer->value() * 1000); //converts to seconds before emiting value
	});

	dataLoggingLayout->addWidget(data_logging_timer);
	dataLoggingLayout->addWidget(dataLoggerFilter);

	warningMessage = new QLabel("");
	warningMessage->setStyleSheet("color:white;");
	warningMessage->setWordWrap(true);
	warningMessage->setVisible(false);

	dataLoggingLayout->addWidget(warningMessage);

	//on data logging switch pressed enable/disable data logging section and emit data loggin toggled
	connect(dataLoggingSwitch,  &CustomSwitch::toggled, this, [=](bool toggled){
		Q_EMIT toggleDataLogger(toggled);
		enableDataLoggerFields(toggled);
	});

}

void DataLoggerView::chooseFile()
{
	QString selectedFilter;

	filename = QFileDialog::getSaveFileName(this,
											tr("Export"), "", tr("Comma-separated values files (*.csv);;All Files(*)"),
											&selectedFilter, QFileDialog::Options());
	dataLoggingFilePath->setText(filename);
}


QWidget* DataLoggerView::getDataLoggerViewWidget()
{
	return dataLoggingWidget;
}


bool DataLoggerView::isDataLoggingOn()
{
	return dataLoggingSwitch->isChecked();
}

bool DataLoggerView::isOverwrite()
{
	return overwriteRadio->isChecked();
}

QString DataLoggerView::getFilter()
{
	return dataLoggerFilter->currentText();
}

void DataLoggerView::setWarningMessage(QString message)
{
	warningMessage->setText(message);
}

void DataLoggerView::enableDataLoggerFields(bool en)
{
	dataLoggingFilePath->setDisabled(!en);
	dataLoggingBrowseBtn->setDisabled(!en);
	overwriteRadio->setDisabled(!en);
	appendRadio->setDisabled(!en);
	data_logging_timer->setDisabled(!en);
	dataLoggerFilter->setDisabled(!en);
	warningMessage->setVisible(en);

	if (filename.isEmpty() && dataLoggingFilePath->isEnabled()) {
		dataLoggingFilePath->setText(tr("No file selected"));
		dataLoggingFilePath->setStyleSheet("color:red");
	}
}

void DataLoggerView::disableDataLogging(bool en)
{
	dataLoggingSwitch->setEnabled(!en);
}

void DataLoggerView::toggleDataLoggerSwitch(bool toggle)
{
	dataLoggingSwitch->setChecked(toggle);
}

void DataLoggerView::setDataLoggerPath(QString path)
{
	filename = path;
	dataLoggingFilePath->setText(path);
}

QString DataLoggerView::getDataLoggerPath()
{
	return filename;
}

void DataLoggerView::setOverwrite(bool en)
{
	if (en) {
		overwriteRadio->setChecked(true);
	} else {
		appendRadio->setChecked(true);
	}
}

int DataLoggerView::getTimerInterval()
{
	return data_logging_timer->value();
}

void DataLoggerView::setTimerInterval(int interval)
{
	data_logging_timer->setValue(interval);
}

void DataLoggerView::isDataLoggerRunning(bool en)
{
	overwriteRadio->setDisabled(en);
	appendRadio->setDisabled(en);
}

DataLoggerView::~DataLoggerView()
{
	delete dataLoggingWidget;
	delete dataLoggingLayout;
	delete dataLoggingFilePath;
	delete overwriteRadio;
	delete appendRadio;
	delete dataLoggingSwitch;
	delete dataLoggerFilter;
	delete dataLoggingBrowseBtn;
}

