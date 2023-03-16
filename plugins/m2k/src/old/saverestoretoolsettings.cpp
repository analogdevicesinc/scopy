#include "saverestoretoolsettings.h"
#include <pluginbase/apiobject.h>
#include <QSettings>

using namespace adiscope::m2k;

SaveRestoreToolSettings::SaveRestoreToolSettings(M2kTool *tool)
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
