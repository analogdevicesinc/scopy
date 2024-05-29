#include "cursorcomponent.h"
#include <menucontrolbutton.h>

using namespace scopy::adc;
using namespace scopy;

CursorComponent::CursorComponent(TimePlotComponent *plot, ToolTemplate *tool, QObject *parent)
	: QObject(parent)
	, ToolComponent()
	, m_plot(plot)
	{

	cursor = new MenuControlButton();
	setupCursorButtonHelper(cursor);

	cursorController = new CursorController(m_plot->timePlot(), this);
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
	cursorController->getPlotCursors()->getX1Cursor()->setPosition(0);
	cursorController->getPlotCursors()->getX2Cursor()->setPosition(0);
	cursorController->getPlotCursors()->getY1Cursor()->setPosition(0);
	cursorController->getPlotCursors()->getY2Cursor()->setPosition(0);
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
