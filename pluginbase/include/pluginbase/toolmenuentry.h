#ifndef TOOLMENUENTRY_H
#define TOOLMENUENTRY_H

#include <QObject>
#include <QString>
#include "scopypluginbase_export.h"

namespace adiscope {
class SCOPYPLUGINBASE_EXPORT ToolMenuEntry : public QObject {
	Q_OBJECT
public:
	ToolMenuEntry(QString id, QString name, QString icon, QObject *parent = nullptr) :
		QObject(parent), m_id(id), m_name(name), m_icon(icon),
		m_enabled(false), m_running(false), m_runBtnVisible(false), m_tool(nullptr) {}

	~ToolMenuEntry() {}
	QString id() { return m_id; }
	QString name() { return m_name; }
	QString icon() { return m_icon; }
	bool enabled() { return m_enabled; }
	bool running() { return m_running; }
	bool runBtnVisible() { return m_runBtnVisible; }
	QWidget *tool() { return m_tool; }

	void setName(const QString &newName);
	void setIcon(const QString &newIcon);
	void setEnabled(bool newEnabled);
	void setRunning(bool newRunning);
	void setRunBtnVisible(bool newRunBtnVisible);
	void setTool(QWidget *newTool);

Q_SIGNALS:
	void updateToolEntry();
	void updateTool();

private:
	QString m_id;
	QString m_name;
	QString m_icon;
	bool m_enabled;
	bool m_running;
	bool m_runBtnVisible;
	QWidget* m_tool;
};

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
