#ifndef TOOL_VIEW_RECIPIE_HPP
#define TOOL_VIEW_RECIPIE_HPP

#include "channel_manager.hpp"
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
	bool hasRightMenu{false};
};

}
}

#endif // TOOL_VIEW_RECIPIE_HPP
