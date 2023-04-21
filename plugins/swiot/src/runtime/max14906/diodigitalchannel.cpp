#include "diodigitalchannel.h"
#include <QPixmap>
#include <QDebug>

using namespace scopy::swiot;

DioDigitalChannel::DioDigitalChannel(const QString& deviceName, const QString& deviceType, QWidget *parent) :
	ui(new Ui::DioDigitalChannel()),
	m_deviceName(deviceName),
	m_deviceType(deviceType) {

	this->ui->setupUi(this);

	this->ui->m_channelName->setText(deviceName);
	this->ui->m_channelType->setText(deviceType);

        // shrink pixmap for better looks
        m_pixmapSize = QPixmap(":/swiot/ic_hi_snow.svg").size() - QSize(2, 2);

        m_highSnow = QPixmap(":/swiot/ic_hi_snow.svg").scaled(m_pixmapSize, Qt::KeepAspectRatio);
        m_highGray = QPixmap(":/swiot/ic_hi_gray.svg").scaled(m_pixmapSize, Qt::KeepAspectRatio);
        m_lowSnow = QPixmap(":/swiot/ic_lo_snow.svg").scaled(m_pixmapSize, Qt::KeepAspectRatio);
        m_lowGray = QPixmap(":/swiot/ic_lo_gray.svg").scaled(m_pixmapSize, Qt::KeepAspectRatio);

	this->ui->customSwitch->setOn(m_highSnow);
	this->ui->customSwitch->setOff(m_lowGray);

	if (deviceType == "input") {
		this->ui->customSwitch->setVisible(false);
	}

	this->ui->scaleChannel->setOrientation(Qt::Horizontal);
	this->ui->scaleChannel->setScalePosition(QwtThermo::LeadingScale);
	this->ui->scaleChannel->setOriginMode(QwtThermo::OriginCustom);
	this->ui->scaleChannel->setMinimumSize(100, 50);
	this->ui->scaleChannel->setAutoScaler(true);
	this->ui->scaleChannel->setScale(0, 1);
	this->ui->scaleChannel->setAutoScaler(false);
	this->ui->scaleChannel->setScaleStepSize(1);
	this->ui->scaleChannel->setOrientation(Qt::Horizontal);
	this->ui->scaleChannel->setScalePosition(QwtThermo::LeadingScale);
	this->ui->scaleChannel->setOriginMode(QwtThermo::OriginCustom);
	this->ui->scaleChannel->setStyleSheet("font-size:16px;");

	this->ui->sismograph->setPlotAxisXTitle(""); // clear title
	this->ui->sismograph->setAxisScale(0, 0, 1, 1); // y axis
	this->ui->sismograph->setAutoscale(false);
	this->ui->sismograph->setColor(Qt::red);
        this->ui->sismograph->setNumSamples(0);
	this->ui->sismograph->updateYScale(10, 0);
        this->ui->sismograph->setUpdatesEnabled(true);

	this->connectSignalsAndSlots();
	this->ui->lcdNumber->setPrecision(0);
}

DioDigitalChannel::~DioDigitalChannel() {
	delete ui;
}

void DioDigitalChannel::connectSignalsAndSlots() {
	connect(this->ui->customSwitch, &CustomSwitch::toggled, this, [this] (){
                bool isChecked = this->ui->customSwitch->isChecked();
                if (isChecked) { // swap the icons
                        this->ui->customSwitch->setOn(m_highSnow);
                        this->ui->customSwitch->setOff(m_lowGray);
                } else {
                        this->ui->customSwitch->setOn(m_highGray);
                        this->ui->customSwitch->setOff(m_lowSnow);
                }

		Q_EMIT this->outputValueChanged(isChecked);
	});
}

Ui::DioDigitalChannel *DioDigitalChannel::getUi() const {
	return ui;
}

void DioDigitalChannel::updateTimeScale(double newMin, double newMax) {
        this->ui->sismograph->updateYScale(newMax, newMin);
}

void DioDigitalChannel::addDataSample(double value) {
	this->ui->sismograph->plot(value); // TODO: delete these, testing only
	this->ui->sismograph->plot(value);
	this->ui->sismograph->plot(value);
	this->ui->sismograph->plot(value);
	this->ui->sismograph->plot(value);
	this->ui->scaleChannel->setValue(value);
	this->ui->lcdNumber->display(value);
}

const std::vector<std::string> &DioDigitalChannel::getConfigModes() const {
	return m_configModes;
}

void DioDigitalChannel::setConfigModes(const std::vector<std::string> &configModes) {
	m_configModes = configModes;

	this->ui->configModes->clear();
	for (const std::string& item : this->m_configModes) {
		this->ui->configModes->addItem(QString::fromStdString(item));
	}
}

const QString &DioDigitalChannel::getSelectedConfigMode() const {
	return m_selectedConfigMode;
}

void DioDigitalChannel::setSelectedConfigMode(const QString &selectedConfigMode) {
	m_selectedConfigMode = selectedConfigMode;

	int idx = this->ui->configModes->findText(selectedConfigMode);
	this->ui->configModes->setCurrentIndex(idx);
	qDebug() << "The channel " << this->m_deviceName << " read selected config mode " << selectedConfigMode;
}

void DioDigitalChannel::resetSismograph() {
	this->ui->sismograph->reset();
}

#include "moc_diodigitalchannel.cpp"
