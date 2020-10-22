#ifndef NEWINSTRUMENT_HPP
#define NEWINSTRUMENT_HPP

#include <filter.hpp>
#include <tool_launcher.hpp>
#include "ui_newinstrument.h"
#include <tool.hpp>

namespace adiscope {
class NewInstrument : public Tool
{
	Ui::NewInstrument *ui;
public:
	NewInstrument(struct iio_context *ctx, Filter *filt,
		      ToolMenuItem *toolMenuItem,
		      QJSEngine *engine, ToolLauncher *parent);
	~NewInstrument();

};
}
#endif // NEWINSTRUMENT_HPP
