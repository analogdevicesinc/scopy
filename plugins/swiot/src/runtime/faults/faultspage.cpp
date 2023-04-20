#include "faultspage.hpp"
#include "ui_faultspage.h"

using namespace scopy::swiot;

FaultsPage::FaultsPage(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::FaultsPage),
	m_ad74413rFaultsDevice(new FaultsDevice("ad74413r", ":/swiot/swiot_faults.json", this)),
	m_max14906FaultsDevice(new FaultsDevice("max14906", ":/swiot/swiot_faults.json", this))
{
	ui->setupUi(this);

	this->m_ad74413rFaultsDevice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	this->m_max14906FaultsDevice->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	this->ui->mainVerticalLayout->insertWidget(1, this->m_ad74413rFaultsDevice);
	this->ui->mainVerticalLayout->insertWidget(2, this->m_max14906FaultsDevice);
}

FaultsPage::~FaultsPage() {
	delete ui;
}

void FaultsPage::update(uint32_t ad74413r_faults, uint32_t max14906_faults) {
	this->m_ad74413rFaultsDevice->update(ad74413r_faults);
	this->m_max14906FaultsDevice->update(max14906_faults);
}
