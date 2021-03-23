#include "dynamic_widget.hpp"

#include "ui_cursors_settings.h"

#include <scopy/gui/cursors_settings.hpp>

using namespace scopy::gui;

CursorsSettings::CursorsSettings(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::CursorsSettings)
{
	m_ui->setupUi(this);

	m_ui->widgetVerticalSubsSpr->setLabel("VERTICAL");
	m_ui->widgetVerticalSubsSpr->setButton(false);

	m_ui->widgetHorizontalSubsSpr->setLabel("HORIZONTAL");
	m_ui->widgetHorizontalSubsSpr->setButton(false);

	m_ui->widgetCursorReadoutsSubsSpr->setLabel("CURSOR READOUTS");
	m_ui->widgetCursorReadoutsSubsSpr->setButton(false);

	m_ui->widgetCursorsMenuHeader->setLabel("Cursors");
	m_ui->widgetCursorsMenuHeader->setLineColor(new QColor("#4A64FF"));

	// TODO: Add all the functionalities
	this->setCursorReadoutsVisible(true);
}

CursorsSettings::~CursorsSettings() { delete m_ui; }

void CursorsSettings::setCursorReadoutsVisible(bool visible) { m_ui->widgetReadouts->setVisible(visible); }
