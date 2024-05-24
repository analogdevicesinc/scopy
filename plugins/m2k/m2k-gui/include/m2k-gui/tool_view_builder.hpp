#ifndef TOOLBUILDER_HPP
#define TOOLBUILDER_HPP

#include "scopy-m2k-gui_export.h"
#include "tool_view.hpp"

namespace scopy {
namespace m2kgui {

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
	bool hasHamburgerMenuBtn{true};
};

class SCOPY_M2K_GUI_EXPORT ToolViewBuilder
{
public:
	ToolViewBuilder(const ToolViewRecipe &recipe, ChannelManager *channelManager = nullptr,
			QWidget *parent = nullptr);

	ToolView *build();

private:
	ToolView *m_toolView;
};
} // namespace m2kgui
} // namespace scopy

#endif // TOOLVIEWBUILDER_HPP
