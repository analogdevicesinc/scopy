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
#include "sourceblock.h"

namespace scopy {
namespace acq {

// SourceBlock that generates one channel by evaluating a user-supplied formula.
// Variable T = sample index (0 .. bufferSize-1).
// Output key: <id>_out_math
class SCOPY_CORE_EXPORT MathSource : public SourceBlock
{
	Q_OBJECT
public:
	explicit MathSource(const QString &id, QObject *parent = nullptr);
	~MathSource() override = default;

	void acquire(DataStore *store) override;

	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

	void setFormula(const QString &formula);
	FormulaEvaluator &evaluator() { return m_evaluator; }
	const DataKey &outputKey() const { return m_outputKey; }

	// Writes one derived key, not the per-channel raw keys SourceBlock assumes.
	QList<DataKey> outputKeys() const override { return {m_outputKey}; }

private:
	FormulaEvaluator m_evaluator;
	DataKey m_outputKey;
};

} // namespace acq
} // namespace scopy
