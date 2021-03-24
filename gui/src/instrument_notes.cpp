#include "ui_instrument_notes.h"

#include <scopy/gui/instrument_notes.hpp>

using namespace scopy::gui;

InstrumentNotes::InstrumentNotes(QWidget* parent)
	: QWidget(parent)
	, m_ui(new Ui::InstrumentNotes)
{
	m_ui->setupUi(this);

	m_ui->widgetSubsectionSeparator->setLabel("Note");
	m_ui->widgetSubsectionSeparator->setButton(false);
}

InstrumentNotes::~InstrumentNotes() { delete m_ui; }

QString InstrumentNotes::getNotes() { return m_ui->textEdit->toPlainText(); }

void InstrumentNotes::setNotes(const QString& str) { m_ui->textEdit->setPlainText(str); }
