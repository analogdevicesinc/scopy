#ifndef ICHANNELPLUGIN_H
#define ICHANNELPLUGIN_H

#include <QObject>
#include <QWidget>

#include "gui/tool_view.hpp"

namespace adiscope {
//namespace gui {
class IChannelPlugin : public QObject
{
	Q_OBJECT
public:
	explicit IChannelPlugin(QWidget *parent = nullptr, gui::ToolView* toolView = nullptr, gui::ChannelManager *chManager = nullptr);
	~IChannelPlugin();

	virtual void init();

	QWidget *parent;
	gui::ToolView* toolView;
	gui::ChannelManager* chManager;
	gui::GenericMenu* menu;

Q_SIGNALS:

};
}
//}
#endif // ICHANNELPLUGIN_H
