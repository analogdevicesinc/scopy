#ifndef TESTTOOL_HPP
#define TESTTOOL_HPP

#include <scopy/gui/tool_view_builder.hpp>

namespace scopy {
namespace gui {

class TestTool : public QWidget
{
	Q_OBJECT
public:
	TestTool();

	ToolView* getToolView();

private:
	ToolView* m_toolView;
};
} // namespace gui
} // namespace scopy

#endif // TESTTOOL_HPP
