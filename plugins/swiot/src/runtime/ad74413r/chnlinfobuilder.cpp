#include "chnlinfobuilder.h"
#include "src/runtime/ad74413r/currentchnlinfo.h"
#include "resistancechnlinfo.h"
#include "voltagechnlinfo.h"

using namespace scopy::swiot;

ChnlInfo* ChnlInfoBuilder::build(iio_channel *iioChnl, QString id)
{
	int chnl_type = decodeId(id);
	switch (chnl_type) {
	case VOLTAGE:
		return new VoltageChnlInfo("V", "mV", iioChnl);
	case CURRENT:
		return new CurrentChnlInfo("mA", "mA", iioChnl);
	case RESISTANCE:
		return new ResistanceChnlInfo("Ω", "Ω", iioChnl);
	default:
		return nullptr;
	}
}
