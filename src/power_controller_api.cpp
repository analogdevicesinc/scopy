#include "power_controller_api.hpp"
#include "ui_powercontrol.h"

namespace adiscope {
void PowerController_API::show()
{
	Q_EMIT pw->showTool();
}

bool PowerController_API::syncEnabled() const
{
	return !pw->ui->btnSync->isChecked();
}

void PowerController_API::enableSync(bool en)
{
	pw->ui->btnSync->setChecked(en);
	pw->ui->btnSync->click();
}

int PowerController_API::getTrackingPercent() const
{
	return pw->ui->trackingRatio->value();
}

void PowerController_API::setTrackingPercent(int percent)
{
	pw->ui->trackingRatio->setValue(percent);
}

double PowerController_API::valueDac1() const
{
	return pw->valuePos->value();
}

void PowerController_API::setValueDac1(double value)
{
	pw->valuePos->setValue(value);
}

double PowerController_API::valueDac2() const
{
	return pw->valueNeg->value();
}

void PowerController_API::setValueDac2(double value)
{
	if (!syncEnabled())
		pw->valueNeg->setValue(value);
}

bool PowerController_API::Dac1Enabled() const
{
	return pw->ui->dac1->isChecked();
}

void PowerController_API::setDac1Enabled(bool enable)
{
	pw->ui->dac1->setChecked(enable);
}

bool PowerController_API::Dac2Enabled() const
{
	return pw->ui->dac2->isChecked();
}

void PowerController_API::setDac2Enabled(bool enable)
{
	pw->ui->dac2->setChecked(enable);
}
}
