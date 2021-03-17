#include "ui_subsection_separator.h"

#include <QHBoxLayout>

#include <scopy/gui/subsection_separator.hpp>

using namespace scopy::gui;

SubsectionSeparator::SubsectionSeparator(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SubsectionSeparator)
{
	m_ui->setupUi(this);
}

SubsectionSeparator::SubsectionSeparator(const QString& text, const bool buttonVisible, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SubsectionSeparator)
{
	m_ui->setupUi(this);

	setLabel(text);
	setButton(buttonVisible);
}

SubsectionSeparator::~SubsectionSeparator() { delete m_ui; }

QPushButton* SubsectionSeparator::getButton() { return m_ui->btnSubsectionSeparator; }

void SubsectionSeparator::setButton(bool buttonVisible) { m_ui->btnSubsectionSeparator->setVisible(buttonVisible); }

QLabel* SubsectionSeparator::getLabel() { return m_ui->lblSubsectionSeparator; }

void SubsectionSeparator::setLabel(const QString& text) { m_ui->lblSubsectionSeparator->setText(text); }
