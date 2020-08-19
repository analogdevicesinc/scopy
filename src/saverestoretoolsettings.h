#ifndef SAVERESTORETOOLSETTINGS_H
#define SAVERESTORETOOLSETTINGS_H

#include "tool.hpp"

#include <QTemporaryFile>
#include <QtGlobal>

namespace adiscope {
class SaveRestoreToolSettings
{
	Q_DISABLE_COPY_MOVE(SaveRestoreToolSettings)
public:
	SaveRestoreToolSettings(Tool *tool);
	~SaveRestoreToolSettings();

private:
	QTemporaryFile m_temp;
	Tool *m_tool;
};
} // namespace adiscope

#endif // SAVERESTORETOOLSETTINGS_H
