#ifndef SCOPYCONFIG_H
#define SCOPYCONFIG_H

#include <QString>
#include "scopy-pluginbase_export.h"

namespace scopy {

class SCOPY_PLUGINBASE_EXPORT config {
public:
	static QString defaultPluginFolderPath();
	static QString localPluginFolderPath();
	static QString preferencesFolderPath();
	static QString settingsFolderPath();
	static QString executableFolderPath();
	static QString version();
	static QString gitCommit();
	static QString fullversion();
	static QString os();
	static QString pcSpecs();

	static QString dump();
};
}
#endif // SCOPYCONFIG_H
