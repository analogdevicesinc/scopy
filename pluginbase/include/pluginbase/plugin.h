#ifndef PLUGIN_H
#define PLUGIN_H

#include <QString>
#include <QWidget>
#include "toolmenuentry.h"
#include "scopypluginbase_export.h"
#include <QtPlugin>
#include <QSettings>
#include <QAbstractButton>
#include "apiobject.h"

namespace adiscope {

/**
 * @brief The Plugin interface
 * Implement this interface to make a plugin loadable in scopy-core
 * To avoid boilerplate code, default implementations are available for most of the methods in PluginBase
 * This makes most of the methods optional, with the exception of
 * compatible()
 * onConnect()
 * onDisconnect()
 */
class SCOPYPLUGINBASE_EXPORT Plugin {
public:
	virtual ~Plugin() {}

	/**
	 * @brief setParam
	 * sets parameters used by the plugin
	 * Default implementation in PluginBase - override not recommended
	 * @param QString
	 */

	virtual void setParam(QString) = 0;

	/**
	 * @brief initMetadata
	 * initializes plugin metadata. initialize m_metadata
	 * use loadMetadata() helper to convert QString to QJsonObject.
	 * Default implementation in PluginBase - sets a default priority - can be overriden
	 *
	 * Metadata is a json object with the following values:
	 * "priority": int - handles the order in which plugins are loaded
	 * "category": list of strings - for which categories this plugin should be considered
	 * "exclude": list of strings - if this plugin is loaded, exclude the plugins in the list.
	 * Can be set to ["*"] - everything - or ["*","!iio"] - everything except iio.
	 */
	virtual void initMetadata() = 0;

	/**
	 * @brief setMetadata
	 * @param obj
	 * Default implementation in PluginBase - override not recommended
	 * sets object metadata to the object
	 * Boilerplate implementation in PluginBase
	 */
	virtual void setMetadata(QJsonObject obj) = 0;

	/**
	 * @brief init
	 * is called at scopy init. Can be used to initialize resources on Scopy startup
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual void init() = 0;

	/**
	 * @brief deinit
	 * is called at scopy shutdown. Can be used to deinit resources on Scopy shutdown
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual void deinit() = 0;

	/**
	 * @brief version
	 * @return plugin version
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual QString version() = 0;

	/**
	 * @brief about
	 * @return QString in markdown format - used to populate Scopy about page
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual QString about() = 0;

	/**
	 * @brief initPreferences
	 * Called at scopy init. Initializes plugin preferences
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual void initPreferences() = 0;

	/**
	 * @brief loadPreferencesPage
	 * is called at scopy init. initialize m_preferencesPage. This widget is added to scopy's preferences page.
	 * @return bool if page is loaded
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual bool loadPreferencesPage() = 0;


	/**
	 * @brief compatible
	 * @param param
	 * @return bool if plugin is compatible with the received parameters
	 *
	 * is called from the original plugin instance.
	 * If the compatible method returns true, a new plugin instance will be created using
	 * clone() and clone extra
	 * IMPORTANT !!! This method MUST NOT alter object state
	 * because since it is called from original plugin instance, the state will not be
	 * maintained in the device's plugin instance.
	 *
	 * No default implementation in PluginBase - must be overriden
	 */
	virtual bool compatible(QString param) = 0;

	/**
	 * @brief clone
	 * returns a new instance of Plugin
	 * @return
	 * Default implementation in SCOPY_PLUGIN macro - override not recomended
	 */
	virtual Plugin* clone() = 0;

	/**
	 * @brief cloneExtra
	 * copies extra information from the parameter plugin
	 * which is not part of the boilerplate implementation
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual void cloneExtra(Plugin*) = 0;

	/**
	 * @brief preload
	 * is called first thing after succesful compatible.
	 * This method is not called from the UI thread
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual void preload() = 0;

	/**
	 * @brief loadApi
	 * is called postcompatible. initialize m_pluginApi to have save/load/js in your plugin
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual void loadApi() = 0;

	/**
	 * @brief loadIcon
	 * is called postcompatible. initialize m_icon widget. This widget is considered as a device icon.
	 * @return bool if icon is loaded
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual bool loadIcon() = 0;

	/**
	 * @brief loadPage
	 * is called postcompatible. initialize m_page widget. This widget is added to the device infopage
	 * @return bool if page is loaded
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual bool loadPage() = 0;
	/**
	 * @brief loadToolList
	 * is called postcompatible. initialize m_toolList. m_toolList is used to populate tool menu
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual void loadToolList() = 0;

	/**
	 * @brief loadExtraButtons
	 * @return is called postcompatible. initialize m_extraButtons. m_extraButtons is used to add buttons with
	 * extra functionality on top of the infopage. Buttons can be added to regular m_page widget as well.
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual bool loadExtraButtons() = 0;

	/**
	 * @brief postload
	 * is called postcompatible after all widget loads are finished
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual void postload() = 0;

	/**
	 * @brief saveSettings
	 * can be called postconnect, on device disconnect. Saves m_pluginApi to the default location
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual void saveSettings() = 0;
	/**
	 * @brief loadSettings
	 * can be called postconnect, on device connect. Loads m_pluginApi from the default location
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual void loadSettings() = 0;

	/**
	 * @brief saveSettings
	 * can be called postconnect, when user requests save. Saves m_plugin to QSettings
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual void saveSettings(QSettings&) = 0;

	/**
	 * @brief loadSettings
	 * can be called postconnect, when user requests load. Loads m_plugin from QSettings
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual void loadSettings(QSettings&) = 0;

	/**
	 * @brief api
	 * @return
	 * getter for m_pluginApi
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual ApiObject* api() = 0;

	/**
	 * @brief unload
	 * called before device destroy's it's plugin instance
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual void unload() = 0;

	/**
	 * @brief param
	 * @return plugin m_param getter - parameters used to instantiate the plugin
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual QString param() = 0;

	/**
	 * @brief nam
	 * @return plugin m_name getter
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual QString name() = 0;

	/**
	 * @brief displayName
	 * @return plugin m_displayName getter - name used by the plugin in Scopy Ui
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual QString displayName() = 0;

	/**
	 * @brief displayDescription
	 * @return plugin m_displayDescription getter - description used by the plugin in Scopy Ui
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual QString displayDescription() = 0;

	/**
	 * @brief icon
	 * @return plugin m_icon getter - icon widget
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual QWidget* icon() = 0;

	/**
	 * @brief page
	 * @return plugin m_page getter - plugin infopage widget. used to create device infopage
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual QWidget* page() = 0;

	/**
	 * @brief preferencesPage
	 * @return plugin m_preferencesPage getter - preferences widget
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual QWidget* preferencesPage() = 0;

	/**
	 * @brief extraButtons
	 * @return plugin m_extraButtons getter - list of buttons used to create device infopage
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual QList<QAbstractButton*> extraButtons() = 0;

	/**
	 * @brief toolList
	 * @return plugin m_toolList getter - list of tools used to populate device toolmenu
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual QList<adiscope::ToolMenuEntry*> toolList() = 0;

	/**
	 * @brief metadata
	 * @return plugin m_metadata getter
	 * Default implementation in PluginBase - override not recommended
	 */
	virtual QJsonObject metadata() = 0;

public Q_SLOTS:
	/**
	 * @brief onConnect
	 * is called when scopy connects to the device - usually by user clicking the connect button.
	 * Initialize tools by calling
	 * m_toolList[]->setEnable(true)
	 * m_toolList[]->setTool(instrument)
	 * @return
	 * No default implementation in PluginBase - must be overriden
	 */
	virtual bool onConnect() = 0;
	/**
	 * @brief onDisconnect
	 * is called when scopy disconnects from the device
	 * deinitialize tools
	 * m_toolList[]->setEnable(false)
	 * m_toolList[]->setTool(nullptr)
	 * @return
	 * No default implementation in PluginBase - must be overriden
	 */
	virtual bool onDisconnect() = 0;

	/**
	 * @brief showPageCallback
	 * is called postcompatible. when the Device infopage comes in foreground (by selecting the device in the DeviceBrowser)
	 * useful for interactive infopages
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual void showPageCallback() = 0;

	/**
	 * @brief hidePageCallback
	 * is called postcompatible when the device infopage goes in the background (by selecting another device in the DeviceBrowser)
	 * useful for interactive infopages
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual void hidePageCallback() = 0;

	/**
	 * @brief messageCallback
	 * is called when the messageBroker sends a message to this plugin
	 * @param topic
	 * @param message
	 * Default implementation in PluginBase - can be overriden
	 */
	virtual void messageCallback(QString topic, QString message) = 0;
Q_SIGNALS:

	/**
	 * @brief disconnectDevice
	 * Plugin can emit this signal if it requests device disconnect
	 * For example when the device is no longer available
	 */
	virtual void disconnectDevice() = 0;

	/**
	 * @brief requestTool
	 * Plugin can emit this signal if it requests bringing a tool in the foreground	 *
	 */
	virtual void requestTool(QString) = 0;

	/**
	 * @brief toolListChanged
	 * Plugin can emit this signal if there are runtime changes to the toollist structure
	 * (adding/removing tools after postload)
	 */
	virtual void toolListChanged() = 0;
	virtual void restartDevice() = 0;


};
}

#define ScopyPlugin_iid "org.adi.Scopy.Plugins.Pluginbase/0.1"
Q_DECLARE_INTERFACE(adiscope::Plugin, ScopyPlugin_iid)

#endif // PLUGIN_H
