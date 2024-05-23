#include "searchbar.h"
#include <QHBoxLayout>
#include "stylehelper.h"

using namespace scopy::iiodebugplugin;

SearchBar::SearchBar(QSet<QString> options, QWidget *parent)
	: QWidget(parent)
	, m_lineEdit(new QLineEdit(this))
	, m_label(new QLabel("Filter", this))
	, m_completer(new QCompleter(options.values(), this))
{
	StyleHelper::BackgroundPage(this, "SearchBar");

	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setFilterMode(Qt::MatchContains);
	m_lineEdit->setCompleter(m_completer);

	setLayout(new QHBoxLayout(this));
	layout()->addWidget(m_label);
	layout()->addWidget(m_lineEdit);
}

QLineEdit *SearchBar::getLineEdit() { return m_lineEdit; }
