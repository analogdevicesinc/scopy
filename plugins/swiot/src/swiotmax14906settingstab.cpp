#include "swiotmax14906settingstab.h"
#include "ui_swiotmax14906settingstab.h"

Max14906SettingsTab::Max14906SettingsTab(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Max14906SettingsTab)
{
	ui->setupUi(this);


	this->m_max_spin_button = new PositionSpinButton({
								 {tr("s"), 1}
								 //								{tr("m"), 60}
							 }, tr("Time"), 0, 300, true, false, this);
	this->m_max_spin_button->setValue(10); // default value
	this->ui->verticalLayout_maxTimer->addWidget(this->m_max_spin_button);

	connect(m_max_spin_button, &PositionSpinButton::valueChanged, [&] () {
		Q_EMIT timeValueChanged(this->m_max_spin_button->value());
	});
}

Max14906SettingsTab::~Max14906SettingsTab()
{
	delete ui;
}

double Max14906SettingsTab::getTimeValue() const {
	return m_max_spin_button->value();
}
