#ifndef SCOPYCONFIG_H
#define SCOPYCONFIG_H

#include "scopy-common_export.h"

#include <QString>

namespace scopy {

class SCOPY_COMMON_EXPORT config
{
public:
	static QString defaultPluginFolderPath();
	static QString localPluginFolderPath();
	static QString defaultTranslationFolderPath();
	static QString localTranslationFolderPath();
	static QString preferencesFolderPath();
	static QString settingsFolderPath();
	static QString executableFolderPath();
	static QString version();
	static QString gitCommit();
	static QString fullversion();
	static QString os();
	static QString pcSpecs();

	static QString dump();
	static QString getUuid();

private:
	inline static int uuid = 0;
};
} // namespace scopy
#endif // SCOPYCONFIG_H
