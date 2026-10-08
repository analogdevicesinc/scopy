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

#pragma once

#include "scopy-core_export.h"

#include "datakey.h"
#include "formulaevaluator.h"
#include "processorblock.h"

namespace scopy {
namespace acq {

// ProcessorBlock that transforms one or more input channels by evaluating a
// user-supplied formula.
// Variables exposed to the formula:
//   S        = sample index (0 .. N-1)
//   X1..XN   = current sample of each watched input (in setWatchedKeys() order)
// Output key is set via setOutputKey(); inputs via setWatchedKeys().
class SCOPY_CORE_EXPORT MathProcessor : public ProcessorBlock
{
	Q_OBJECT
public:
	explicit MathProcessor(const QString &name, QObject *parent = nullptr);
	~MathProcessor() override = default;

	void setOutputKey(const DataKey &k) { m_outputKey = k; }
	const DataKey &outputKey() const { return m_outputKey; }

	QList<DataKey> outputKeys() const override { return {m_outputKey}; }

	void setWatchedKeys(const QList<DataKey> &keys) override;

	void process(DataStore *store) override;
	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

	void setFormula(const QString &formula);
	FormulaEvaluator &evaluator() { return m_evaluator; }

Q_SIGNALS:
	void inputsChanged();

private:
	FormulaEvaluator m_evaluator;
	DataKey m_outputKey;
};

} // namespace acq
} // namespace scopy
