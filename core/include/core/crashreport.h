#ifndef CRASHREPORT_H
#define CRASHREPORT_H

#include <QString>
#include "scopy-core_export.h"

namespace scopy {
class SCOPY_CORE_EXPORT CrashReport
{
public:
	static void initSignalHandler();

private:
	static void signalHandler(int);
	static QString tmpFilePath_;
};
} // namespace scopy

#endif // CRASHREPORT_H
