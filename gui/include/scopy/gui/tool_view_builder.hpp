#ifndef TOOLBUILDER_HPP
#define TOOLBUILDER_HPP

#include <scopy/gui/tool_view.hpp>

namespace scopy {
namespace gui {

struct ToolViewRecipe
{
	QString url{""};
	bool hasHelpBtn{true};
	bool hasPrintBtn{false};
	bool hasGroupBtn{false};

	QWidget* hasExtraWidget{nullptr};

	bool hasRunBtn{false};
	bool hasSingleBtn{false};

	bool hasPairSettingsBtn{false};

	bool hasCursors{false};
	bool hasTrigger{false};
	bool hasMeasure{false};
	bool hasSweep{false};
	bool hasMarkers{false};
	bool hasChannelSettings{false};

	bool hasInstrumentNotes{false};

	bool hasChannels{false};
	bool hasAddMathBtn{false};
};

class ToolViewBuilder
{
public:
	ToolViewBuilder(const ToolViewRecipe& recipe);

	ToolView* build();

private:
	ToolView* m_toolView;
};
} // namespace gui
} // namespace scopy

#endif // TOOLVIEWBUILDER_HPP
