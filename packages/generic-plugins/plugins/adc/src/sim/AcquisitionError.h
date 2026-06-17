#pragma once

#include <QString>

namespace scopy {
namespace adc {
namespace sim {

struct AcquisitionError
{
	enum class Severity { Info, Warning, Critical };

	Severity severity;
	QString  id;
	QString  message;
};

} // namespace sim
} // namespace adc
} // namespace scopy
