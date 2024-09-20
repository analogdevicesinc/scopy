#include <baseheaderwidget.h>
#include <stylehelper.h>

using namespace scopy;

BaseHeaderWidget::BaseHeaderWidget(QString title, QWidget *parent)
	: BaseHeader(parent)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	lay->setMargin(0);
	lay->setSpacing(0);

	m_label = new QLineEdit(title, this);
	m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_label->setEnabled(false);
	m_label->setReadOnly(false);
	StyleHelper::MenuCollapseHeaderLineEdit(m_label, "menuCollapseLabel");
	m_label->setTextMargins(0, 0, 0, 0);

	lay->addWidget(m_label);
}

BaseHeaderWidget::~BaseHeaderWidget() {}

void BaseHeaderWidget::setTitle(QString title) { m_label->setText(title); }

QString BaseHeaderWidget::title() { return m_label->text(); }
