#include "progresslineedit.h"

using namespace scopy;

ProgressLineEdit::ProgressLineEdit(QWidget *parent)
	: QWidget(parent)
	, m_lineEdit(new QLineEdit(this))
	, m_progressBar(new SmallProgressBar(this))
{
	StyleHelper::MenuLineEdit(m_lineEdit, "lineEdit");
	m_progressBar->setStyleSheet("background-color: grey;");

	setLayout(new QVBoxLayout(this));
	layout()->addWidget(m_lineEdit);
	layout()->addWidget(m_progressBar);
	layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Expanding));
	layout()->setContentsMargins(0, 0, 0, 0);
	layout()->setSpacing(0);

	connect(m_lineEdit, &QLineEdit::editingFinished, this, [this]() { m_progressBar->startProgress(); });
}

SmallProgressBar *ProgressLineEdit::getProgressBar() { return m_progressBar; }

QLineEdit *ProgressLineEdit::getLineEdit() { return m_lineEdit; }

#include "moc_progresslineedit.cpp"
