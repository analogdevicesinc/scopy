#include "cursorcomponent.h"
#include <menucontrolbutton.h>

using namespace scopy::adc;
using namespace scopy;

CursorComponent::CursorComponent(PlotComponent *plot, ToolTemplate *tool, QObject *parent) : QObject(parent), m_plot(plot) {

	cursor = new MenuControlButton();
	setupCursorButtonHelper(cursor);	

	cursorController = new CursorController(m_plot->plot(), this);
	HoverWidget *hoverSettings = new HoverWidget(cursorController->getCursorSettings(), cursor, tool);
	hoverSettings->setAnchorPos(HoverPosition::HP_TOPRIGHT);
	hoverSettings->setContentPos(HoverPosition::HP_TOPLEFT);
	hoverSettings->setAnchorOffset(QPoint(0, -10));


	connect(cursor->button(), &QAbstractButton::toggled, hoverSettings, &HoverWidget::setVisible);
	connect(cursor, &QAbstractButton::toggled, cursorController, &CursorController::setVisible);
	tool->addWidgetToBottomContainerHelper(cursor, TTA_RIGHT);
}

CursorComponent::~CursorComponent()
{

}

void CursorComponent::onInit() {
	cursorController->getPlotCursors()->getH1Cursor()->setPosition(0);
	cursorController->getPlotCursors()->getH2Cursor()->setPosition(0);
	cursorController->getPlotCursors()->getV1Cursor()->setPosition(0);
	cursorController->getPlotCursors()->getV2Cursor()->setPosition(0);
}



MenuControlButton *CursorComponent::ctrl()
{
	return cursor;
}


void CursorComponent::setupCursorButtonHelper(MenuControlButton *cursor)
{
	cursor->setName("Cursors");
	cursor->setOpenMenuChecksThis(true);
	cursor->setDoubleClickToOpenMenu(true);
	cursor->checkBox()->setVisible(false);
	cursor->setCheckBoxStyle(MenuControlButton::CS_SQUARE);
}
