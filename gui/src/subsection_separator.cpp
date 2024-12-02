#include "subsection_separator.hpp"

#include "ui_subsection_separator.h"

#include <QHBoxLayout>

using namespace scopy::gui;

SubsectionSeparator::SubsectionSeparator(QWidget *parent)
	: QWidget(parent)
	, m_ui(new Ui::SubsectionSeparator)
{
	m_ui->setupUi(this);

	m_ui->btnSubsectionSeparator->setChecked(true);
	connect(m_ui->btnSubsectionSeparator, &QPushButton::toggled,
		[=](bool toggled) { m_ui->widgetSubsectionContent->setVisible(toggled); });
}

SubsectionSeparator::SubsectionSeparator(const QString &text, const bool buttonVisible, QWidget *parent)
	: SubsectionSeparator(parent)
{
	setLabel(text);
	setButtonVisible(buttonVisible);
}

SubsectionSeparator::~SubsectionSeparator() { delete m_ui; }

QPushButton *SubsectionSeparator::getButton() { return m_ui->btnSubsectionSeparator; }

void SubsectionSeparator::setButtonVisible(bool buttonVisible)
{
	m_ui->btnSubsectionSeparator->setVisible(buttonVisible);
}

bool SubsectionSeparator::getButtonChecked() { return m_ui->btnSubsectionSeparator->isChecked(); }

void SubsectionSeparator::setButtonChecked(bool checked) { m_ui->btnSubsectionSeparator->setChecked(checked); }

QLabel *SubsectionSeparator::getLabel() { return m_ui->lblSubsectionSeparator; }

void SubsectionSeparator::setLabel(const QString &text) { m_ui->lblSubsectionSeparator->setText(text); }

void SubsectionSeparator::setLabelVisible(bool visible) { m_ui->lblSubsectionSeparator->setVisible(visible); }

void SubsectionSeparator::setLineVisible(bool visible) { m_ui->lineSubsectionSeparator->setVisible(visible); }

void SubsectionSeparator::setContent(QWidget *content) { m_ui->vLayoutContent->addWidget(content); }

QWidget *SubsectionSeparator::getContentWidget() { return m_ui->widgetSubsectionContent; }

#include "moc_subsection_separator.cpp"
