#ifndef RESISTANCECHNLINFO_H
#define RESISTANCECHNLINFO_H
#include "chnlinfo.h"

#define RPULL_UP 2100
#define ADC_MAX_VALUE 65535
#define MIN_RESISTANCE_VALUE 0
#define MAX_RESISTANCE_VALUE 1000000000
namespace scopy::swiotrefactor {
class ResistanceChnlInfo : public ChnlInfo
{
	Q_OBJECT
public:
	explicit ResistanceChnlInfo(QString plotUm = "Ω", QString hwUm = "Ω", iio_channel *iioChnl = nullptr,
				    CommandQueue *cmdQueue = nullptr);
	~ResistanceChnlInfo();

	double convertData(unsigned int data) override;
};
} // namespace scopy::swiotrefactor
#endif // RESISTANCECHNLINFO_H
