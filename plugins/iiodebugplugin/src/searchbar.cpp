#include "searchbar.h"
#include <QHBoxLayout>

using namespace scopy::iiodebugplugin;

SearchBar::SearchBar(const QStringList &options, QWidget *parent)
	: QWidget(parent)
	, m_lineEdit(new QLineEdit(this))
	, m_label(new QLabel("Filter", this))
	, m_completer(new QCompleter(options, this))
{
	m_completer->setCaseSensitivity(Qt::CaseInsensitive);
	m_completer->setFilterMode(Qt::MatchContains);
	m_lineEdit->setCompleter(m_completer);

	setLayout(new QHBoxLayout(this));
	layout()->addWidget(m_label);
	layout()->addWidget(m_lineEdit);
	setStyleSheet("color: white; background-color: #272730");
	m_label->setStyleSheet("color: white; background-color: #272730");
}

QLineEdit *SearchBar::getLineEdit() { return m_lineEdit; }
