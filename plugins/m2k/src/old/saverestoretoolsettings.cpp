#include "saverestoretoolsettings.h"

#include <QSettings>

#include <pluginbase/apiobject.h>

using namespace scopy::m2k;

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
