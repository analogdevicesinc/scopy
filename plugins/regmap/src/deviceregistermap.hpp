#ifndef DEVICEREGISTERMAP_HPP
#define DEVICEREGISTERMAP_HPP

#include "scopy-regmap_export.h"

#include <QMap>
#include <QObject>
#include <QWidget>
#include <tooltemplate.h>
#include "scopy-regmap_export.h"

class QVBoxLayout;

class QDockWidget;

namespace scopy::regmap {
class RegisterMapTemplate;
class RegisterModel;
class RegisterMapValues;
class RegisterDetailedWidget;
class RegisterController;
class SearchBarWidget;
class RegisterMapTable;

class SCOPY_REGMAP_EXPORT DeviceRegisterMap : public QWidget
{
	friend class RegMap_API;
	Q_OBJECT
public:
	explicit DeviceRegisterMap(RegisterMapTemplate *registerMapTemplate = nullptr,
				   RegisterMapValues *registerMapValues = nullptr, QWidget *parent = nullptr);
	~DeviceRegisterMap();

	void registerChanged(RegisterModel *regModel);
	void toggleAutoread(bool toggled);
	void applyFilters(QString filter);
	bool hasTemplate();
	bool getAutoread();

private:
	ToolTemplate *tool;
	bool autoread = false;
	QVBoxLayout *layout;
	RegisterMapTemplate *registerMapTemplate;
	RegisterMapValues *registerMapValues;
	RegisterController *registerController = nullptr;

	RegisterMapTable *registerMapTableWidget = nullptr;
	QDockWidget *docRegisterMapTable = nullptr;

	RegisterDetailedWidget *registerDetailedWidget = nullptr;
	void initSettings();
	int selectedRegister;

Q_SIGNALS:
	void requestRead(uint32_t address);
	void requestWrite(uint32_t address, uint32_t value);
	void requestRegisterDump(QString path);
};
} // namespace scopy::regmap
#endif // DEVICEREGISTERMAP_HPP
