#ifndef LOGICTOOL_H
#define LOGICTOOL_H

#include "tool.hpp"

namespace adiscope {
namespace logic {
class LogicTool : public Tool
{
	Q_OBJECT
public:
	LogicTool(struct iio_context *ctx, ToolMenuItem *toolMenuItem,
	          ApiObject *api, const QString& name,
	          ToolLauncher *parent);
	virtual ~LogicTool() = default;

	uint16_t *getData();

Q_SIGNALS:
	void dataAvailable(uint64_t, uint64_t);

protected:
	uint16_t *m_buffer;
};
} // namespace logic
} // namespace adiscope

#endif // LOGICTOOL_H
