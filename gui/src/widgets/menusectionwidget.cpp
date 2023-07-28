#include <widgets/menusectionwidget.h>
#include <stylehelper.h>

using namespace scopy;
MenuSectionWidget::MenuSectionWidget(QWidget *parent) : QWidget(parent) {
	m_layout = new QVBoxLayout(this);

	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	setLayout(m_layout);
	m_layout->setSpacing(0);
	m_layout->setMargin(0);
	StyleHelper::MenuSectionWidget(this, "menuSection");
}

MenuSectionWidget::~MenuSectionWidget() { }

QVBoxLayout *MenuSectionWidget::contentLayout() const { return m_layout; }
