#ifndef SAVERESTORETOOLSETTINGS_H
#define SAVERESTORETOOLSETTINGS_H

#include "tool.hpp"

#include <QTemporaryFile>
#include <QtGlobal>

namespace adiscope {
class SaveRestoreToolSettings
{
	SaveRestoreToolSettings(const SaveRestoreToolSettings &) = delete;
	SaveRestoreToolSettings &operator=(const SaveRestoreToolSettings &) = delete;

	SaveRestoreToolSettings(SaveRestoreToolSettings &&) = delete;
	SaveRestoreToolSettings &operator=(SaveRestoreToolSettings &&) = delete;
public:
	SaveRestoreToolSettings(Tool *tool);
	~SaveRestoreToolSettings();

private:
	QTemporaryFile m_temp;
	Tool *m_tool;
};
} // namespace adiscope

#endif // SAVERESTORETOOLSETTINGS_H
