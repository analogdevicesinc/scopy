#include "MathBlockWidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

namespace scopy {
namespace adc {

MathBlockWidget::MathBlockWidget(sim::FormulaEvaluator *evaluator, QWidget *parent)
	: QWidget(parent)
{
	auto *lay    = new QVBoxLayout(this);
	auto *row    = new QHBoxLayout();
	auto *label  = new QLabel("Formula:", this);
	auto *edit   = new QLineEdit(evaluator->formula(), this);
	auto *status = new QLabel(this);

	status->setFixedWidth(20);

	auto updateStatus = [status, evaluator]() {
		if(evaluator->isValid()) {
			status->setText("\u2713");
			status->setStyleSheet("color: green;");
		} else {
			status->setText("\u2717");
			status->setStyleSheet("color: red;");
		}
	};
	updateStatus();

	row->addWidget(label);
	row->addWidget(edit);
	row->addWidget(status);
	lay->addLayout(row);
	lay->addStretch();

	connect(edit, &QLineEdit::textChanged, this, [evaluator, updateStatus](const QString &text) {
		evaluator->setFormula(text);
		updateStatus();
	});
}

} // namespace adc
} // namespace scopy
