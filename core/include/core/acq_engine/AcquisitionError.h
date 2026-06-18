#pragma once

#include <QString>

namespace scopy {
namespace acq {

struct AcquisitionError
{
	enum class Severity { Info, Warning, Critical };

	Severity severity;
	QString  id;
	QString  message;
};

} // namespace acq
} // namespace scopy
