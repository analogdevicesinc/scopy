#include "faultsdevice.h"
#include <utility>
#include "faultsgroup.h"
#include "src/swiot_logging_categories.h"

#define FAULT_CHANNEL_NAME "voltage"


using namespace scopy::swiot;

FaultsDevice::FaultsDevice(const QString& name, QString path, struct iio_device* device, struct iio_device* swiot, QWidget *parent)
	: ui(new Ui::FaultsDevice),
	  QWidget(parent),
	  m_faults_explanation(new QWidget(this)),
	  m_subsectionSeparator(new scopy::gui::SubsectionSeparator("Faults Explanation", true, this)),
	  m_name(name.toUpper()),
	  m_path(std::move(path)),
	  m_device(device),
	  m_swiot(swiot)
{
	m_faultsGroup = new FaultsGroup(name, m_path, this->getSpecialFaults(), this);

	ui->setupUi(this);
	this->connectSignalsAndSlots();
	this->initFaultExplanations();

	this->ui->lineEdit_numeric->setPlaceholderText("0x0");
	this->ui->lineEdit_numeric->setFocusPolicy(Qt::NoFocus);

	setDynamicProperty(m_subsectionSeparator->getButton(), "subsection_arrow_button", true);
	m_subsectionSeparator->setContent(m_faults_explanation);

	this->ui->label_name->setText(m_name);
	this->ui->lineEdit_numeric->setReadOnly(true);
	this->ui->faults_layout->addWidget(this->m_faultsGroup);
	this->ui->faults_explanation->layout()->addWidget(m_subsectionSeparator);
	m_faults_explanation->ensurePolished();
}

FaultsDevice::~FaultsDevice() {
	delete ui;
}

void FaultsDevice::resetStored() {
	for (auto fault: this->m_faultsGroup->getFaults()) {
		fault->setStored(false);
	}
	this->updateExplanations();
}

void FaultsDevice::update() {
	this->readFaults();
	this->ui->lineEdit_numeric->setText(QString("0x%1").arg(m_faultNumeric, 8, 16, QLatin1Char('0')));
	this->m_faultsGroup->update(m_faultNumeric);
	this->updateExplanations();
}

void FaultsDevice::updateExplanations() {
	std::set<unsigned int> selected = m_faultsGroup->getSelectedIndexes();
	std::set<unsigned int> actives = m_faultsGroup->getActiveIndexes();
	if (selected.empty()) {
		for (int i = 0; i < m_faultExplanationWidgets.size(); ++i) {
			m_faultExplanationWidgets[i]->show();

			if (actives.contains(i)) {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", true);
			} else {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", false);
			}
		}
	} else {
		for (int i = 0; i < m_faultExplanationWidgets.size(); ++i) {
			if (selected.contains(i)) {
				m_faultExplanationWidgets[i]->show();
			} else {
				m_faultExplanationWidgets[i]->hide();
			}

			if (actives.contains(i)) {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", true);
			} else {
				setDynamicProperty(m_faultExplanationWidgets[i], "highlighted", false);
			}
		}
	}

	m_faults_explanation->ensurePolished();
}

void FaultsDevice::updateMinimumHeight() {
	this->ensurePolished();
	this->m_faults_explanation->ensurePolished();
	this->m_faultsGroup->ensurePolished();
}

void FaultsDevice::initFaultExplanations() {
	m_faults_explanation->setLayout(new QVBoxLayout(m_faults_explanation));
	m_faults_explanation->layout()->setContentsMargins(0, 0, 0, 0);
	m_faults_explanation->layout()->setSpacing(0);
	m_faults_explanation->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	m_faults_explanation->layout()->setSizeConstraint(QLayout::SetMinimumSize);

	QStringList l = this->m_faultsGroup->getExplanations();
	for (const auto &item: l) {
		auto widget = new QLabel(item, m_faults_explanation);
		widget->setTextFormat(Qt::PlainText);
		widget->setStyleSheet("QWidget[highlighted=true]{color:white;} QWidget{color:#5c5c5c;}");

		widget->setWordWrap(true);
		m_faultExplanationWidgets.push_back(widget);
		m_faults_explanation->layout()->addWidget(widget);
	}
	m_faults_explanation->ensurePolished();
}

void FaultsDevice::connectSignalsAndSlots() {
	connect(this->ui->clear_selection_button, &QPushButton::clicked, this->m_faultsGroup,
		&FaultsGroup::clearSelection);
	connect(this->ui->reset_button, &QPushButton::clicked, this, &FaultsDevice::resetStored);
	connect(m_faultsGroup, &FaultsGroup::selectionUpdated, this, &FaultsDevice::updateExplanations);
	connect(m_faultsGroup, &FaultsGroup::minimumSizeChanged, this, &FaultsDevice::updateMinimumHeight);
}

void FaultsDevice::readFaults() {
	if (m_device == nullptr) {
		qCritical(CAT_SWIOT_FAULTS) << "No device was found";
		return;
	}

	iio_channel *chn = iio_device_find_channel(m_device, FAULT_CHANNEL_NAME, false);
	if (chn == nullptr) {
		qCritical(CAT_SWIOT_FAULTS) << "Channel" << FAULT_CHANNEL_NAME << "was not found";
		return;
	}

	char fau[100];
	ssize_t readResult = iio_channel_attr_read(chn, "raw", fau, 100);

	if (readResult < 0) {
		qCritical(CAT_SWIOT_FAULTS) << m_name << "faults value could not be read.";
	} else {
		qDebug(CAT_SWIOT_FAULTS) << m_name << "faults read the value:" << fau;
		try {
			m_faultNumeric = std::stoi(fau);
		} catch (std::invalid_argument& exception) {
			qCritical(CAT_SWIOT_FAULTS) << m_name << "faults value could not be converted from string to int, read" << fau << "; exception message:" << exception.what();
		}
	}
}

QMap<int, QString>* FaultsDevice::getSpecialFaults() {
	// read the 4 special channels from ad74413r
	auto result = new QMap<int, QString>();
	std::string function, device;

	char buffer[256] = {0};
	iio_device_attr_read(m_swiot, "ch0_device", buffer, 256);

	for (int i = 0; i < 4; ++i) {
		device = "ch" + std::to_string(i) + "_device";
		char readDevice[256] = {0};
		ssize_t readDeviceResult = iio_device_attr_read(m_swiot, device.c_str(), readDevice, 255);
		if (readDeviceResult < 0) {
			qCritical(CAT_SWIOT_FAULTS) << "Error: cannot read swiot property" << QString::fromStdString(device);
		} else {
			if (std::string(readDevice) == "ad74413r") {
				function = "ch" + std::to_string(i) + "_function";
				char readFunction[256] = {0};
				ssize_t readFunctionResult = iio_device_attr_read(m_swiot, function.c_str(), readFunction, 256);
				if (readFunctionResult < 0) {
					qCritical(CAT_SWIOT_FAULTS) << "Error: cannot read swiot property" << QString::fromStdString(function);
				} else {
					result->insert(i, QString(readFunction));
				}
			}
		}
	}

	if (result->isEmpty()) {
		delete result;
		return nullptr;
	}
	return result;
}
