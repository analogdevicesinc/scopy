/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "mathblockwidget.h"

#include "mathprocessor.h"

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
	auto *row = new QHBoxLayout();
	auto *label = new QLabel("Formula:", self);
	auto *edit = new QLineEdit(evaluator->formula(), self);
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

	QObject::connect(edit, &QLineEdit::textChanged, self, [evaluator, updateStatus](const QString &text) {
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
	auto *inputsLay = new QVBoxLayout(inputsContainer);
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
			inputsLay->addWidget(
				new QLabel(QString("X%1 = %2").arg(i + 1).arg(keys[i].toString()), inputsContainer));
		}
	};

	connect(proc, &MathProcessor::inputsChanged, this, rebuildInputs);
	rebuildInputs();

	lay->addStretch();
}

} // namespace acq
} // namespace scopy

#include "moc_mathblockwidget.cpp"
