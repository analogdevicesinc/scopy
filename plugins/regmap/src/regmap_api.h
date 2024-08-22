#ifndef REGMAP_API_H
#define REGMAP_API_H

#include "scopy-regmap_export.h"

#include <regmapplugin.h>
#include <deviceregistermap.hpp>
#include <registermaptool.hpp>
#include <register/registermodel.hpp>
#include <registermapvalues.hpp>
#include "search.hpp"

namespace scopy::regmap {

class SCOPY_REGMAP_EXPORT RegMap_API : public ApiObject
{

	Q_OBJECT

public:
	explicit RegMap_API(RegmapPlugin *regMapPlugin);
	~RegMap_API();

	Q_INVOKABLE void write(QString addr, QString val);
	Q_INVOKABLE QStringList getAvailableDevicesName();
	Q_INVOKABLE bool setDevice(QString device);
	Q_INVOKABLE QStringList search(QString searchParam);
	Q_INVOKABLE QStringList readInterval(QString startAddr, QString stopAddr);
	Q_INVOKABLE bool enableAutoread(bool enable);
	Q_INVOKABLE bool isAutoreadEnabled();
	Q_INVOKABLE void registerDump(QString filePath);
	Q_INVOKABLE void writeFromFile(QString filePath);

private:
	RegmapPlugin *m_regMapPlugin;
	DeviceRegisterMap *getActiveDevRegMap();
};
} // namespace scopy::regmap
#endif // REGMAP_API_H
