#ifndef REGISTERMAPTOOL_HPP
#define REGISTERMAPTOOL_HPP

#include <QObject>
#include <QWidget>
#include <toolbuttons.h>
#include "registermaptemplate.hpp"
#include "scopy-regmap_export.h"
#include "gui/tooltemplate.h"
#include <iio.h>

class QComboBox;
namespace scopy {
namespace regmap {

class RegisterMapValues;

class DeviceRegisterMap;

class RegisterMapSettingsMenu;

class SearchBarWidget;

class SCOPY_REGMAP_EXPORT RegisterMapTool : public QWidget
{
	friend class RegMap_API;
	Q_OBJECT
public:
	explicit RegisterMapTool(QWidget *parent = nullptr);
	~RegisterMapTool();

	void addDevice(QString devName, RegisterMapTemplate *registerMapTemplate = nullptr,
		       RegisterMapValues *registerMapValues = nullptr);

signals:

private:
	ToolTemplate *tool;
	GearBtn *settingsMenu;
	QComboBox *registerDeviceList;
	QString activeRegisterMap;
	SearchBarWidget *searchBarWidget;
	RegisterMapSettingsMenu *settings;
	QMap<QString, DeviceRegisterMap *> *deviceList;
	bool first = true;
	void toggleSettingsMenu(QString registerName, bool toggle);

private Q_SLOTS:
	void updateActiveRegisterMap(QString registerName);
	void toggleSearchBarEnabled(bool enabled);
};
} // namespace regmap
} // namespace scopy
#endif // REGISTERMAPTOOL_HPP
