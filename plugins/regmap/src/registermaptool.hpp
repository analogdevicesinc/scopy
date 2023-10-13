#ifndef REGISTERMAPTOOL_HPP
#define REGISTERMAPTOOL_HPP

#include <QObject>
#include <QWidget>
#include <toolbuttons.h>
#include "scopy-regmapplugin_export.h"
#include "gui/tooltemplate.h"
#include <iio.h>

class QComboBox;
using namespace scopy;

class TabInfo;
namespace scopy::regmap {

class RegisterMapValues;

class DeviceRegisterMap;

class RegisterMapSettingsMenu;

class SearchBarWidget;
class SCOPY_REGMAPPLUGIN_EXPORT RegisterMapTool : public QWidget
{
	Q_OBJECT
public:
	explicit RegisterMapTool(QWidget *parent = nullptr);
	~RegisterMapTool();

	void addTab(struct iio_device *dev, QString title, QString xmlPath);
	void addTab(struct iio_device *dev, QString title);
	void addTab(QString filePath, QString title);

signals:

private:
	ToolTemplate *tool;
	GearBtn *settingsMenu;
	QComboBox *registerDeviceList;
	QString activeRegisterMap;
	SearchBarWidget *searchBarWidget;
	scopy::regmap::RegisterMapSettingsMenu *settings;
	QMap<QString, TabInfo *> *tabsInfo;
	QMap<QString, DeviceRegisterMap *> *tabs;
	bool first = true;

	RegisterMapValues *getRegisterMapValues(struct iio_device *dev);
	RegisterMapValues *getRegisterMapValues(QString filePath);
	void generateDeviceRegisterMap(TabInfo *tabInfo);
	void toggleSettingsMenu(QString registerName, bool toggle);

private Q_SLOTS:
	void updateActiveRegisterMap(QString registerName);
	void toggleSearchBarVisible(bool visible);
};
} // namespace scopy::regmap
#endif // REGISTERMAPTOOL_HPP
