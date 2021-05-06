#include "ui_subsection_separator.h"

#include <QHBoxLayout>

#include <scopy/gui/subsection_separator.hpp>

using namespace scopy::gui;

SubsectionSeparator::SubsectionSeparator(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SubsectionSeparator)
{
	m_ui->setupUi(this);

	connect(m_ui->btnSubsectionSeparator, &QPushButton::toggled,
		[=](bool toggled) { m_ui->widgetSubsectionContent->setVisible(toggled); });
}

SubsectionSeparator::SubsectionSeparator(const QString& text, const bool buttonVisible, QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::SubsectionSeparator)
{
	m_ui->setupUi(this);

	setLabel(text);
	setButtonVisible(buttonVisible);
}

SubsectionSeparator::~SubsectionSeparator() { delete m_ui; }

QPushButton* SubsectionSeparator::getButton() { return m_ui->btnSubsectionSeparator; }

void SubsectionSeparator::setButtonVisible(bool buttonVisible)
{
	m_ui->btnSubsectionSeparator->setVisible(buttonVisible);
}

bool SubsectionSeparator::getButtonChecked() { return m_ui->btnSubsectionSeparator->isChecked(); }

void SubsectionSeparator::setButtonChecked(bool checked) { m_ui->btnSubsectionSeparator->setChecked(checked); }

QLabel* SubsectionSeparator::getLabel() { return m_ui->lblSubsectionSeparator; }

void SubsectionSeparator::setLabel(const QString& text) { m_ui->lblSubsectionSeparator->setText(text); }

void SubsectionSeparator::setLabelVisible(bool visible) { m_ui->lblSubsectionSeparator->setVisible(visible); }

void SubsectionSeparator::setLineVisible(bool visible) { m_ui->lineSubsectionSeparator->setVisible(visible); }
