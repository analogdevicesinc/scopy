#ifndef CURSORCOMPONENT_H
#define CURSORCOMPONENT_H

#include <menucontrolbutton.h>
#include <gui/cursorcontroller.h>
#include "timeplotcomponent.h"

namespace scopy {
namespace adc {
class CursorComponent : public QObject, public ToolComponent
{
	Q_OBJECT
public:
	CursorComponent(PlotComponent *plot, ToolTemplate *tool, QObject *parent = nullptr);
	~CursorComponent();

	void onInit() override;
	void onDeinit() override {}
	void onStart() override {}
	void onStop() override {}

	MenuControlButton *ctrl();

private:

	PlotComponent *m_plot;
	CursorController *cursorController;
	MenuControlButton *cursor;

	void setupCursorButtonHelper(MenuControlButton *cursor);

};
}
}
#endif // CURSORCOMPONENT_H
