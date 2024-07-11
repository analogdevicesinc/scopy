#include <QVBoxLayout>

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
}

MenuComboWidget::~MenuComboWidget() {}

QComboBox *MenuComboWidget::combo() { return m_combo->combo(); }

MenuCombo::MenuCombo(QString title, QWidget *parent)
	: QWidget(parent)
{
	auto lay = new QVBoxLayout(this);
	m_mouseWheelGuard = new MouseWheelWidgetGuard(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(0);

	m_label = new QLabel(title, this);
	m_combo = new QComboBox(this);

	lay->addWidget(m_label);
	lay->addWidget(m_combo);
	m_mouseWheelGuard->installEventRecursively(this);
}

MenuCombo::~MenuCombo() {}
QComboBox *MenuCombo::combo() { return m_combo; }

#include "moc_menucombo.cpp"
