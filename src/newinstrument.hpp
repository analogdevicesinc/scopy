#ifndef NEWINSTRUMENT_HPP
#define NEWINSTRUMENT_HPP

#include <filter.hpp>
#include "ui_newinstrument.h"
#include <tool.hpp>
#include <gui/tool_view.hpp>
#include <gui/tool_view_recipie.hpp>


namespace adiscope {

class IRightMenuPlugin;
class IChannelPlugin;

namespace gui {

class RightMenuManager;
class GenericMenu;
class ChannelManager;
class DataLoggerGenericMenu;
}


class NewInstrument : public Tool
{
	Q_OBJECT
	Ui::NewInstrument *ui;
public:
	explicit NewInstrument(struct iio_context *ctx, Filter *filt,
		      ToolMenuItem *toolMenuItem,
		      QJSEngine *engine, ToolLauncher *parent);
	~NewInstrument();

	void initGeneralSettings();
	void addPlugins();
private:
	ToolLauncher *m_parent;
	gui::ToolViewRecipe m_recipe;
	gui::ToolView* m_toolView;
	gui::GenericMenu* m_generalSettingsMenu;
	gui::ChannelManager* m_channelManager;
	std::vector<IChannelPlugin *> m_channelPluginList;
	std::vector<IRightMenuPlugin *> m_rightMenuPluginList;
};
}
#endif // NEWINSTRUMENT_HPP
