#include "widgets/menulineedit.h"

using namespace scopy;

MenuLineEdit::MenuLineEdit(QWidget *parent)
	: QWidget(parent)
{
	auto lay = new QHBoxLayout(this);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(0);

	m_edit = new QLineEdit(this);

	lay->addWidget(m_edit);
	applyStylesheet();
}

MenuLineEdit::~MenuLineEdit() {}

QLineEdit *MenuLineEdit::edit() { return m_edit; }

void MenuLineEdit::applyStylesheet()
{
	StyleHelper::MenuLineEdit(m_edit, "menuButton");
	StyleHelper::MenuLineEditWidget(this, "menuBigSwitch");
}

#include "moc_menulineedit.cpp"
