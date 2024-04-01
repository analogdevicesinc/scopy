#include "edittextmenuheader.h"

#include <stylehelper.h>

using namespace scopy;

scopy::EditTextMenuHeader::EditTextMenuHeader(QString title, QPen pen, QWidget *parent)
	: QWidget(parent)
{
	auto lay = new QVBoxLayout(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(3);

	m_lineEdit = new QLineEdit(title, this);
	m_line = new QFrame(this);
	m_pen = pen;

	lay->addWidget(m_lineEdit);
	lay->addWidget(m_line);
	applyStylesheet();
}

scopy::EditTextMenuHeader::~EditTextMenuHeader() {}

QLineEdit *EditTextMenuHeader::lineEdit() const { return m_lineEdit; }

void EditTextMenuHeader::applyStylesheet()
{
	StyleHelper::MenuHeaderLine(m_line, m_pen, "menuSeparator");
	StyleHelper::MenuEditTextHeaderWidget(this, "menuHeader");
}

#include "moc_edittextmenuheader.cpp"
