#include "ui_subsection_separator.h"

#include <QHBoxLayout>

#include <scopy/gui/subsection_separator.hpp>

using namespace scopy::gui;

SubsectionSeparator::SubsectionSeparator(const QString& text, const bool buttonVisible, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SubsectionSeparator)
{
	m_ui->setupUi(this);

	m_ui->lblSubsectionSeparator->setText(text);
	m_ui->btnSubsectionSeparator->setVisible(buttonVisible);
}

SubsectionSeparator::~SubsectionSeparator() { delete m_ui; }

QPushButton* SubsectionSeparator::getButton() { return m_ui->btnSubsectionSeparator; }

QLabel* SubsectionSeparator::getLabel() { return m_ui->lblSubsectionSeparator; }
