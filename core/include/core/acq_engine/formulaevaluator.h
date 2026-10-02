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

#include <atomic>
#include <memory>
#include <mutex>

#include <QJSEngine>
#include <QJSValue>
#include <QString>
#include <QVector>

namespace scopy {
namespace acq {

class SCOPY_CORE_EXPORT FormulaEvaluator
{
public:
	FormulaEvaluator();
	~FormulaEvaluator();

	void setFormula(const QString &formula); // GUI thread
	bool isValid() const;			 // GUI thread
	QString formula() const;		 // GUI thread

	// Worker thread. Inputs are exposed in JS as X1, X2, ..., XN (N = inputs.size()).
	// The sample index is available as S. A null pointer in `inputs` is treated
	// as a 0-valued input. On failure returns false, fills `out` with zeros and
	// writes a human-readable reason into `errorOut` (if non-null).
	bool evaluateBatch(int n, const QVector<const QVector<float> *> &inputs, QVector<float> &out,
			   QString *errorOut = nullptr);

private:
	static QString wrapFormula(const QString &formula, int inputCount);
	void rebuildEngine(const QString &formula, int inputCount);

	mutable std::mutex m_mutex;
	QString m_formula;
	bool m_dirty{true};
	std::atomic<bool> m_syntaxValid{false};
	int m_lastInputCount{0};

	std::unique_ptr<QJSEngine> m_workerEngine;
	QJSValue m_batchFn;

	const QString m_setupScript;
};

} // namespace acq
} // namespace scopy
