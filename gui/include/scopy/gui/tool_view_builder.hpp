#ifndef TOOLBUILDER_HPP
#define TOOLBUILDER_HPP

#include <scopy/gui/tool_view.hpp>

namespace scopy {
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
};

class ToolViewBuilder
{
public:
	ToolViewBuilder(const ToolViewRecipe& recipe, ChannelManager* channelManager = nullptr);

	ToolView* build();

private:
	ToolView* m_toolView;
};
} // namespace gui
} // namespace scopy

#endif // TOOLVIEWBUILDER_HPP
