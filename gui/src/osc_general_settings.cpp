#include "ui_osc_general_settings.h"

#include <scopy/gui/osc_general_settings.hpp>

using namespace scopy::gui;

OscGeneralSettings::OscGeneralSettings(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::OscGeneralSettings)
{
	m_ui->setupUi(this);

	m_ui->widgetGeneralSettingsMenuHeader->setLabel("General settings");
	m_ui->widgetGeneralSettingsMenuHeader->setLineColor(new QColor("#4a64ff"));

	m_ui->widgetXYSubsSep->setLabel("X-Y");
	m_ui->widgetXYSubsSep->setButtonVisible(false);

	m_ui->widgetViewsSubsSep->setLabel("VIEWS");
	m_ui->widgetViewsSubsSep->setButtonVisible(false);
}

OscGeneralSettings::~OscGeneralSettings() { delete m_ui; }
