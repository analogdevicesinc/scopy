#include "diosettingstab.h"
#include "ui_swiotmax14906settingstab.h"

using namespace scopy::swiot;

DioSettingsTab::DioSettingsTab(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::DioSettingsTab) {
	ui->setupUi(this);

	this->m_max_spin_button = new PositionSpinButton({
								 {tr("s"), 1}
							 }, tr("Timespan"), 0, 300, true, false, this);
	this->m_max_spin_button->setValue(10); // default value
	this->ui->verticalLayout_maxTimer->addWidget(this->m_max_spin_button);

	QObject::connect(m_max_spin_button, &PositionSpinButton::valueChanged, [this]() {
		Q_EMIT timeValueChanged(this->m_max_spin_button->value());
	});
}

DioSettingsTab::~DioSettingsTab() {
	delete ui;
}

double DioSettingsTab::getTimeValue() const {
	return m_max_spin_button->value();
}
