#ifndef ADCFFTINSTRUMENTCONTROLLER_H
#define ADCFFTINSTRUMENTCONTROLLER_H

#include "scopy-adc_export.h"
#include "adcinstrumentcontroller.h"

namespace scopy {
namespace adc {

class FFTPlotManagerSettings;
class SCOPY_ADC_EXPORT ADCFFTInstrumentController : public ADCInstrumentController {
public:
	ADCFFTInstrumentController(ToolMenuEntry *tme, QString name, AcqTreeNode *tree, QObject *parent = nullptr);
	~ADCFFTInstrumentController();
	virtual void init() override;
	virtual void addChannel(AcqTreeNode *node) override;
	virtual void removeChannel(AcqTreeNode *node) override;
private:
	FFTPlotManagerSettings* m_fftPlotSettingsComponent;
};

}
}

#endif // ADCFFTINSTRUMENTCONTROLLER_H
