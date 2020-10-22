#include "newinstrument.hpp"

using namespace adiscope;
NewInstrument::NewInstrument(struct iio_context *ctx, Filter *filt,
			     ToolMenuItem *toolMenuItem,
			     QJSEngine *engine, ToolLauncher *parent):
  Tool(ctx, toolMenuItem, nullptr, "NewInstrument",
       parent),
  ui(new Ui::NewInstrument)
{	
	ui->setupUi(this);
	run_button=nullptr;
}

NewInstrument::~NewInstrument()
{
		delete ui;
}
