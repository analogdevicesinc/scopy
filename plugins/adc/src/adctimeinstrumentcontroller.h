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
