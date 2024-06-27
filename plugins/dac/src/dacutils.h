#ifndef DACUTILS_H
#define DACUTILS_H

#include <QObject>
#include "scopy-dac_export.h"

namespace scopy {
namespace dac {
class SCOPY_DAC_EXPORT DacUtils : public QObject
{
	Q_OBJECT
public:
	static double dbFullScaleConvert(double scale, bool inverse);
};
} // namespace dac
} // namespace scopy

#endif // DACUTILS_H
