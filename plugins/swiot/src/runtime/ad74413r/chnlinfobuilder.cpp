#include "chnlinfobuilder.h"

#include "resistancechnlinfo.h"
#include "src/runtime/ad74413r/currentchnlinfo.h"
#include "voltagechnlinfo.h"

using namespace scopy::swiot;

ChnlInfo *ChnlInfoBuilder::build(iio_channel *iioChnl, QString id, CommandQueue *cmdQueue)
{
	int chnl_type = decodeId(id);
	switch(chnl_type) {
	case VOLTAGE:
		return new VoltageChnlInfo("V", "mV", iioChnl, cmdQueue);
	case CURRENT:
		return new CurrentChnlInfo("mA", "mA", iioChnl, cmdQueue);
	case RESISTANCE:
		return new ResistanceChnlInfo("Ω", "Ω", iioChnl, cmdQueue);
	default:
		return nullptr;
	}
}
