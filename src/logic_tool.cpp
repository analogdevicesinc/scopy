#include "logic_tool.h"

using namespace adiscope;
using namespace adiscope::logic;

adiscope::logic::LogicTool::LogicTool(iio_context *ctx, adiscope::ToolMenuItem *toolMenuItem,
                                      adiscope::ApiObject *api, const QString &name,
                                      adiscope::ToolLauncher *parent)
        : Tool(ctx, toolMenuItem, api, name, parent)
        , m_buffer(nullptr)
{

}

uint16_t *LogicTool::getData()
{
	return m_buffer;
}
