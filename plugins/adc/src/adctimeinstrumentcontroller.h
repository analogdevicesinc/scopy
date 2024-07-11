#ifndef ADCTIMEINSTRUMENTCONTROLLER_H
#define ADCTIMEINSTRUMENTCONTROLLER_H

#include "scopy-adc_export.h"
#include "adcinstrumentcontroller.h"

namespace scopy {
namespace adc {
class SCOPY_ADC_EXPORT ADCTimeInstrumentController : public ADCInstrumentController {
public:
	ADCTimeInstrumentController(ToolMenuEntry *tme, QString name, AcqTreeNode *tree, QObject *parent = nullptr);
	~ADCTimeInstrumentController();
	virtual void init() override;
	virtual void addChannel(AcqTreeNode *node) override;
	virtual void removeChannel(AcqTreeNode *node) override;

private:
	TimePlotManagerSettings *m_timePlotSettingsComponent;
};

}
}

#endif // ADCTIMEINSTRUMENTCONTROLLER_H
