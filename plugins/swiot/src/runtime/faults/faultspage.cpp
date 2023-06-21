#include "faultspage.h"
#include "ui_faultspage.h"
#include "src/swiot_logging_categories.h"

using namespace scopy::swiot;

FaultsPage::FaultsPage(struct iio_context* context, QWidget *parent) :
	QWidget(parent),
	m_context(context),
	ui(new Ui::FaultsPage),
	m_ad74413rFaultsDevice(nullptr),
	m_max14906FaultsDevice(nullptr)
{
	this->setupDevices();

	ui->setupUi(this);

	// needed for subsection separator resize
	this->m_ad74413rFaultsDevice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	this->m_max14906FaultsDevice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	this->ui->mainVerticalLayout->addWidget(this->m_ad74413rFaultsDevice);
	this->ui->mainVerticalLayout->addWidget(this->m_max14906FaultsDevice);
	this->ui->mainVerticalLayout->addItem(new QSpacerItem(0 ,0, QSizePolicy::Preferred, QSizePolicy::Expanding));
}

FaultsPage::~FaultsPage() {
	delete ui;
}

void FaultsPage::update() {
	this->m_ad74413rFaultsDevice->update();
	this->m_max14906FaultsDevice->update();
}

void FaultsPage::setupDevices() {
	struct iio_device* ad74413r = iio_context_find_device(m_context, "ad74413r");
	struct iio_device* max14906 = iio_context_find_device(m_context, "max14906");
	struct iio_device* swiot = iio_context_find_device(m_context, "swiot");

	char buffer[256] = {0};
	iio_device_attr_read(swiot, "ch0_device", buffer, 256);

	if (swiot) {
		if (ad74413r) {
			m_ad74413rFaultsDevice = new FaultsDevice("ad74413r", ":/swiot/swiot_faults.json", ad74413r, swiot, this);
		} else {
			qCritical(CAT_SWIOT_FAULTS) << "Error: did not find ad74413r device.";
		}

		if (max14906) {
			m_max14906FaultsDevice = new FaultsDevice("max14906", ":/swiot/swiot_faults.json", max14906, swiot, this);
		} else {
			qCritical(CAT_SWIOT_FAULTS) << "Error: did not find max14906 device.";
		}
	} else {
		qCritical(CAT_SWIOT_FAULTS) << "Error: did not find swiot device.";
	}
}
