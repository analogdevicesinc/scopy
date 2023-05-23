#include "faultspage.h"
#include "ui_faultspage.h"

using namespace scopy::swiot;

FaultsPage::FaultsPage(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::FaultsPage),
	m_ad74413rFaultsDevice(new FaultsDevice("AD74413R", ":/swiot/swiot_faults.json", this)),
	m_max14906FaultsDevice(new FaultsDevice("MAX14906", ":/swiot/swiot_faults.json", this))
{
	ui->setupUi(this);

	// needed for subsection separator resize
	this->m_ad74413rFaultsDevice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	this->m_max14906FaultsDevice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

	this->ui->mainVerticalLayout->insertWidget(0, this->m_ad74413rFaultsDevice);
	this->ui->mainVerticalLayout->insertWidget(1, this->m_max14906FaultsDevice);
}

FaultsPage::~FaultsPage() {
	delete ui;
}

void FaultsPage::update(uint32_t ad74413r_faults, uint32_t max14906_faults) {
	this->m_ad74413rFaultsDevice->update(ad74413r_faults);
	this->m_max14906FaultsDevice->update(max14906_faults);
}
