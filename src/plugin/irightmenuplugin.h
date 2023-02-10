#ifndef IRIGHTMENUPLUGIN_H
#define IRIGHTMENUPLUGIN_H

#include <QObject>
#include <QWidget>

#include "gui/tool_view.hpp"

namespace adiscope {
//namespace gui {
class IRightMenuPlugin : public QObject
{
	Q_OBJECT
public:
	explicit IRightMenuPlugin(QWidget *parent = nullptr, gui::ToolView* toolView = nullptr, bool dockable = false);
	~IRightMenuPlugin();

	virtual void init();

	QWidget *parent;
	gui::ToolView* toolView;
	gui::GenericMenu* menu;
	bool dockable;

Q_SIGNALS:

};
}
//}
#endif // IRIGHTMENUPLUGIN_H
