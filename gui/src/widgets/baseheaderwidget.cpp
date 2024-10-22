#include <baseheaderwidget.h>
#include <style.h>
#include <stylehelper.h>

using namespace scopy;

BaseHeaderWidget::BaseHeaderWidget(QString title, QWidget *parent)
	: QWidget(parent)
{
	m_lay = new QVBoxLayout(this);
	m_lay->setContentsMargins(Style::getDimension(json::global::unit_1), 0, 0, 0);
	m_lay->setSpacing(0);

	m_label = new QLineEdit(title, this);
	m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
	m_label->setEnabled(false);
	m_label->setReadOnly(false);
	StyleHelper::MenuCollapseHeaderLineEdit(m_label, "menuCollapseLabel");

	m_lay->addWidget(m_label);
}

BaseHeaderWidget::~BaseHeaderWidget() {}

void BaseHeaderWidget::setTitle(QString title) { m_label->setText(title); }

QString BaseHeaderWidget::title() { return m_label->text(); }

#include "moc_baseheaderwidget.cpp"
