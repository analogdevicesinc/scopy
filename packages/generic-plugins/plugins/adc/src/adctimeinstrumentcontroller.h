/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef ADCTIMEINSTRUMENTCONTROLLER_H
#define ADCTIMEINSTRUMENTCONTROLLER_H

#include "scopy-adc_export.h"
#include "adcinstrumentcontroller.h"
#include "timeplotmanagersettings.h"

namespace scopy {
namespace adc {
class SCOPY_ADC_EXPORT ADCTimeInstrumentController : public ADCInstrumentController
{
public:
	ADCTimeInstrumentController(ToolMenuEntry *tme, QString uri, QString name, AcqTreeNode *tree,
				    QObject *parent = nullptr);
	~ADCTimeInstrumentController();
	virtual void init() override;
	virtual void addChannel(AcqTreeNode *node) override;
	virtual void removeChannel(AcqTreeNode *node) override;
	void createTimeSink(AcqTreeNode *node);
	void createIIODevice(AcqTreeNode *node);
	void createIIOFloatChannel(AcqTreeNode *node);
	void createImportFloatChannel(AcqTreeNode *node);
	void setEnableAddRemovePlot(bool b) override;

private:
	TimePlotManagerSettings *m_timePlotSettingsComponent;
	ChannelComponent *m_defaultCh;
};

} // namespace adc
} // namespace scopy

#endif // ADCTIMEINSTRUMENTCONTROLLER_H
