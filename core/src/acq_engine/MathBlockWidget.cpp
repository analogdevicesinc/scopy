#include "MathBlockWidget.h"

#include "MathProcessor.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>

namespace scopy {
namespace acq {

namespace {

// Builds the formula line-edit + ✓/✗ row and returns the row layout.
QLayout *buildFormulaRow(QWidget *self, FormulaEvaluator *evaluator)
{
	auto *row    = new QHBoxLayout();
	auto *label  = new QLabel("Formula:", self);
	auto *edit   = new QLineEdit(evaluator->formula(), self);
	auto *status = new QLabel(self);

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

	QObject::connect(edit, &QLineEdit::textChanged, self,
			 [evaluator, updateStatus](const QString &text) {
				 evaluator->setFormula(text);
				 updateStatus();
			 });

	return row;
}

} // namespace

MathBlockWidget::MathBlockWidget(FormulaEvaluator *evaluator, QWidget *parent)
	: QWidget(parent)
{
	auto *lay = new QVBoxLayout(this);
	lay->addLayout(buildFormulaRow(this, evaluator));
	lay->addStretch();
}

MathBlockWidget::MathBlockWidget(MathProcessor *proc, QWidget *parent)
	: QWidget(parent)
{
	auto *lay = new QVBoxLayout(this);
	lay->addLayout(buildFormulaRow(this, &proc->evaluator()));

	auto *inputsContainer = new QWidget(this);
	auto *inputsLay       = new QVBoxLayout(inputsContainer);
	inputsLay->setContentsMargins(0, 0, 0, 0);
	inputsLay->setSpacing(2);
	lay->addWidget(inputsContainer);

	auto rebuildInputs = [proc, inputsContainer, inputsLay]() {
		QLayoutItem *item;
		while((item = inputsLay->takeAt(0)) != nullptr) {
			if(QWidget *w = item->widget())
				w->deleteLater();
			delete item;
		}
		inputsLay->addWidget(new QLabel("S = sample index", inputsContainer));
		const QList<DataKey> &keys = proc->watchedKeys();
		if(keys.isEmpty()) {
			inputsLay->addWidget(new QLabel("(no inputs)", inputsContainer));
			return;
		}
		for(int i = 0; i < keys.size(); ++i) {
			inputsLay->addWidget(new QLabel(
				QString("X%1 = %2").arg(i + 1).arg(keys[i].toString()),
				inputsContainer));
		}
	};

	connect(proc, &MathProcessor::inputsChanged, this, rebuildInputs);
	rebuildInputs();

	lay->addStretch();
}

} // namespace acq
} // namespace scopy
