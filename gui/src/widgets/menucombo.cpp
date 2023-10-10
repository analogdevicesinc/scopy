#include <QVBoxLayout>

#include <stylehelper.h>
#include <widgets/menucombo.h>

using namespace scopy;

MenuComboWidget::MenuComboWidget(QString title, QWidget *parent)
	: QWidget(parent)
{
	auto lay = new QVBoxLayout(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(0);

	m_combo = new MenuCombo(title, this);

	lay->addWidget(m_combo);
	applyStylesheet();
}

MenuComboWidget::~MenuComboWidget() {}

QComboBox *MenuComboWidget::combo() { return m_combo->combo(); }

void MenuComboWidget::applyStylesheet() { StyleHelper::MenuComboWidget(this, "menuComboWidget"); }

MenuCombo::MenuCombo(QString title, QWidget *parent)
	: QWidget(parent)
{
	auto lay = new QVBoxLayout(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(0);

	m_label = new QLabel(title, this);
	m_combo = new QComboBox(this);

	lay->addWidget(m_label);
	lay->addWidget(m_combo);
	applyStylesheet();
}

MenuCombo::~MenuCombo() {}
QComboBox *MenuCombo::combo() { return m_combo; }

void MenuCombo::applyStylesheet()
{
	StyleHelper::MenuComboLabel(m_label, "menuLabel");
	StyleHelper::MenuComboBox(m_combo, "menuCombobox");
}

#include "moc_menucombo.cpp"
