#include "ad74413r/chnlinfobuilder.h"

#include "ad74413r/resistancechnlinfo.h"
#include "ad74413r/currentchnlinfo.h"
#include "ad74413r/voltagechnlinfo.h"
#include "ad74413r/digitalchnlinfo.h"

using namespace scopy::swiotrefactor;

ChnlInfo *ChnlInfoBuilder::build(iio_channel *iioChnl, QString id, CommandQueue *cmdQueue)
{
	int chnl_type = decodeId(id);
	switch(chnl_type) {
	case VOLTAGE:
		return new VoltageChnlInfo("V", "mV", iioChnl, cmdQueue);
	case CURRENT:
		return new CurrentChnlInfo("A", "mA", iioChnl, cmdQueue);
	case RESISTANCE:
		return new ResistanceChnlInfo("Ω", "Ω", iioChnl, cmdQueue);
	case DIGITAL:
		return new DigitalChnlInfo("unit", "unit", iioChnl, cmdQueue);
	default:
		return nullptr;
	}
}
