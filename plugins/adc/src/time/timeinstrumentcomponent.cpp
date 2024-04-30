#include "timeinstrumentcomponent.h"


using namespace scopy;
using namespace adc;

TimeInstrumentComponent::TimeInstrumentComponent(QString name, AcqTreeNode *tree, ADCInstrument *adc, QObject *parent)
{
	chIdP = new ChannelIdProvider(this);
}


