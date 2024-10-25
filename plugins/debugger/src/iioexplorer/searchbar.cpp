#include "searchbar.h"
#include "style_properties.h"
#include <QHBoxLayout>
#include <style.h>

using namespace scopy::debugger;

SearchBar::SearchBar(QSet<QString> options, QWidget *parent)
	: QWidget(parent)
	, m_lineEdit(new QLineEdit(this))
	, m_label(new QLabel("Filter", this))
	, m_completer(new QCompleter(options.values(), this))
{
	Style::setStyle(m_lineEdit, style::properties::debugger::searchBar);
	Style::setStyle(m_label, style::properties::label::menuSmall);
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setFilterMode(Qt::MatchContains);
	m_lineEdit->setPlaceholderText("Type to filter");
	m_lineEdit->setCompleter(m_completer);
	m_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	setLayout(new QHBoxLayout(this));
	layout()->addWidget(m_label);
	layout()->addWidget(m_lineEdit);
	layout()->setMargin(0);
}

QLineEdit *SearchBar::getLineEdit() { return m_lineEdit; }

#include "moc_searchbar.cpp"
