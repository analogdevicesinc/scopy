#include "saverestoretoolsettings.h"

#include <QSettings>

using namespace adiscope;

SaveRestoreToolSettings::SaveRestoreToolSettings(Tool *tool)
        : m_tool(tool)
{
	QSettings settings(m_temp.fileName(), QSettings::IniFormat);

	tool->getApi()->save(settings);

	settings.sync();
}

SaveRestoreToolSettings::~SaveRestoreToolSettings()
{
	QSettings settings(m_temp.fileName(), QSettings::IniFormat);

	m_tool->getApi()->load(settings);
}
