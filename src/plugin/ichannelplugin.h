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
	explicit IChannelPlugin(QWidget *parent = nullptr, gui::ToolView* toolView = nullptr, gui::ChannelManager *chManager = nullptr, bool dockable = false);
	~IChannelPlugin();

	virtual void init();
	virtual void setColor(QColor color);
	virtual std::vector<ChannelWidget*> getChannelList();

	QWidget *parent;
	gui::ToolView* toolView;
	gui::ChannelManager* chManager;
	gui::GenericMenu* menu;
	ChannelWidget *ch;
	std::vector<ChannelWidget*> channelList;
	bool dockable;

public Q_SLOTS:
};
}
//}
#endif // ICHANNELPLUGIN_H
