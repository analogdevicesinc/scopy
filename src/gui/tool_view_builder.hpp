#ifndef TOOLBUILDER_HPP
#define TOOLBUILDER_HPP

#include "tool_view.hpp"
#include <QMainWindow>
#include "tool_launcher.hpp"
#include "tool_view_recipie.hpp"

namespace adiscope {
namespace gui {

class ToolViewBuilder
{
public:
	ToolViewBuilder(const ToolViewRecipe& recipe, ChannelManager* channelManager = nullptr, adiscope::ToolLauncher* parent = nullptr);

	ToolView* build();

private:
	ToolView* m_toolView;
};
} // namespace gui
} // namespace adiscope

#endif // TOOLVIEWBUILDER_HPP
