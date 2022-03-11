#ifndef TOOLBUILDER_HPP
#define TOOLBUILDER_HPP

#include "tool_view.hpp"
#include "tool_launcher.hpp"

namespace adiscope {
namespace gui {

struct ToolViewRecipe
{
	QString helpBtnUrl{""};
	bool hasHelpBtn{true};
	bool hasPrintBtn{false};
	bool hasGroupBtn{false};

	bool hasRunBtn{false};
	bool hasSingleBtn{false};

	bool hasPairSettingsBtn{false};

	bool hasChannels{false};
	ChannelsPositionEnum channelsPosition{ChannelsPositionEnum::HORIZONTAL};
	bool hasHeader{true};
};

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
