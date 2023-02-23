#ifndef IRIGHTMENUPLUGIN_H
#define IRIGHTMENUPLUGIN_H

#include <QObject>
#include <QWidget>

#include "gui/tool_view.hpp"
#include "baseplugin.h"

namespace adiscope {
//namespace gui {
class IRightMenuPlugin : public BasePlugin
{
	Q_OBJECT
protected:
	gui::ToolView* toolView;
	gui::GenericMenu* menu;

public:
	explicit IRightMenuPlugin(QWidget *parent = nullptr, gui::ToolView* toolView = nullptr, bool dockable = false);
	~IRightMenuPlugin();

	void init() override;
};
}
//}
#endif // IRIGHTMENUPLUGIN_H
