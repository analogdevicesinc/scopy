#ifndef ICHANNELPLUGIN_H
#define ICHANNELPLUGIN_H

#include <QObject>
#include <QWidget>

#include "gui/tool_view.hpp"
#include "baseplugin.h"

namespace adiscope {

//namespace gui {
class IChannelPlugin : public BasePlugin
{
	Q_OBJECT
protected:
	gui::ToolView* toolView;
	gui::ChannelManager* chManager;
	gui::GenericMenu* menu;
	ChannelWidget *ch;
	std::vector<ChannelWidget*> channelList;

public:
	explicit IChannelPlugin(QWidget *parent = nullptr, gui::ToolView* toolView = nullptr, gui::ChannelManager *chManager = nullptr, bool dockable = false);
	~IChannelPlugin();

	void init() override;
	virtual void setColor(QColor color);
	virtual std::vector<ChannelWidget*> getChannelList();
};
}
//}
#endif // ICHANNELPLUGIN_H
