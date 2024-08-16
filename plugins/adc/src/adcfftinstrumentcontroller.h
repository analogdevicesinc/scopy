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
	// void setComplexMode(bool b);
	void createIIODevice(AcqTreeNode *node);
	void createIIOFloatChannel(AcqTreeNode *node);
	void createIIOComplexChannel(AcqTreeNode *node_I, AcqTreeNode *node_Q);
	void createFFTSink(AcqTreeNode *node);
	void createImportFloatChannel(AcqTreeNode *node);
	bool getComplexChannelPair(AcqTreeNode *node, AcqTreeNode **node_i, AcqTreeNode **node_q);

private:
	QList<AcqTreeNode*> m_complexChannels;
	FFTPlotManagerSettings* m_fftPlotSettingsComponent;
	ChannelComponent *m_defaultComplexCh, *m_defaultRealCh;

};

}
}

#endif // ADCFFTINSTRUMENTCONTROLLER_H
