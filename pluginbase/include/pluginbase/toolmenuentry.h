#ifndef TOOLMENUENTRY_H
#define TOOLMENUENTRY_H

#include <QObject>
#include <QString>
#include <QUuid>
#include "scopypluginbase_export.h"

namespace adiscope {
/**
 * @brief The ToolMenuEntry class
 * Class used by plugin and scopy to populate the tool menu
 * SCOPY_NEW_TOOLMENUENTRY macro can be used to instantiate ToolMenuEntry objects
 */
class SCOPYPLUGINBASE_EXPORT ToolMenuEntry : public QObject {
	Q_OBJECT
public:
	ToolMenuEntry(QString id, QString name, QString icon, QObject *parent = nullptr) :
		QObject(parent), m_uuid(QUuid::createUuid().toString()), m_id(id), m_name(name), m_icon(icon), m_visible(true),
		m_enabled(false), m_running(false), m_runBtnVisible(false), m_tool(nullptr) {}

	ToolMenuEntry(const ToolMenuEntry &other) {
		m_id = other.m_id;
		m_uuid = other.m_uuid;
		m_name = other.m_name;
		m_icon = other.m_icon;
		m_visible = other.m_visible;
		m_enabled = other.m_enabled;
		m_running = other.m_running;
		m_runBtnVisible = other.m_runBtnVisible;
		m_tool = other.m_tool;
	}

	~ToolMenuEntry() {}
	QString id() { return m_id; }
	QString uuid() { return m_uuid; }
	QString name() { return m_name; }
	QString icon() { return m_icon; }
	bool visible() { return m_visible; }
	bool enabled() { return m_enabled; }
	bool running() { return m_running; }
	bool runBtnVisible() { return m_runBtnVisible; }
	QWidget *tool() { return m_tool; }

	static ToolMenuEntry *findToolMenuEntryByName(QList<ToolMenuEntry*> list, QString id);
	static ToolMenuEntry *findToolMenuEntryById(QList<ToolMenuEntry*> list, QString id);
	/**
	 * @brief setName
	 * @param newName
	 * Set tool menu entry name
	 */
	void setName(const QString &newName);
	/**
	 * @brief setIcon
	 * @param newIcon
	 * Set tool menu entry icon
	 */
	void setIcon(const QString &newIcon);

public Q_SLOTS:
	/**
	 * @brief setVisible
	 * @param newVisible
	 * Makes the menu entry visible
	 */
	void setVisible(bool newVisible);
	/**
	 * @brief setEnabled
	 * @param newEnabled
	 * Enables the toolmenuentry - makes the menu entry clickable
	 */
	void setEnabled(bool newEnabled);
	/**
	 * @brief setRunning
	 * @param newRunning
	 * Sets the tool in run state
	 */
	void setRunning(bool newRunning);
	/**
	 * @brief setRunBtnVisible
	 * @param newRunBtnVisible
	 * Set run button visible on/off
	 */
	void setRunBtnVisible(bool newRunBtnVisible);

	/**
	 * @brief setTool
	 * @param newTool
	 * links a widget to the tool menu entry. Remove tool from the menu entry by setting newtool to nullptr
	 */
	void setTool(QWidget *newTool);

Q_SIGNALS:
	/**
	 * @brief updateToolEntry
	 * signal is emitted automatically when modifying anything about the tool entry
	 */
	void updateToolEntry();
	/**
	 * @brief updateTool
	 * signal is emitted automatically when changing a tool linked to the tool entry
	 */
	void updateTool();

	/**
	 * @brief requestRun
	 * Signal is emitted when the run button is clicked from Scopy UI
	 */
	void runToggled(bool);

private:
	QString m_id;
	QString m_uuid;
	QString m_name;
	QString m_icon;
	bool m_visible;
	bool m_enabled;
	bool m_running;
	bool m_runBtnVisible;
	QWidget* m_tool;
};

inline ToolMenuEntry *ToolMenuEntry::findToolMenuEntryByName(QList<ToolMenuEntry *> list, QString name)
{
	for(auto &&tme : list) {
		if(tme->name()==name) {
			return tme;
		}
	}
	return nullptr;
}

inline ToolMenuEntry *ToolMenuEntry::findToolMenuEntryById(QList<ToolMenuEntry *> list, QString id)
{
	for(auto &&tme : list) {
		if(tme->id()==id) {
			return tme;
		}
	}
	return nullptr;
}

inline void ToolMenuEntry::setName(const QString &newName)
{
	m_name = newName;
	Q_EMIT updateToolEntry();
}

inline void ToolMenuEntry::setIcon(const QString &newIcon)
{
	m_icon = newIcon;
	Q_EMIT updateToolEntry();
}

inline void ToolMenuEntry::setVisible(bool newVisible)
{
	m_visible = newVisible;
	Q_EMIT updateToolEntry();
}

inline void ToolMenuEntry::setEnabled(bool newEnabled)
{
	m_enabled = newEnabled;
	Q_EMIT updateToolEntry();
}

inline void ToolMenuEntry::setRunning(bool newRunning)
{
	m_running = newRunning;
	Q_EMIT updateToolEntry();
}

inline void ToolMenuEntry::setRunBtnVisible(bool newRunBtnVisible)
{
	m_runBtnVisible = newRunBtnVisible;
	Q_EMIT updateToolEntry();
}

inline void ToolMenuEntry::setTool(QWidget *newTool)
{
	m_tool = newTool;
	Q_EMIT updateTool();
}

}

#endif // TOOLMENUENTRY_H
