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

#include "formulaevaluator.h"

#include <QWidget>

namespace scopy {
namespace acq {

class MathProcessor;

// Shared settings widget for MathSource and MathProcessor.
// - FormulaEvaluator overload: just a formula line-edit + ✓/✗.
// - MathProcessor overload: adds a read-only list of the current inputs
//   (X1, X2, … = key) that auto-rebuilds on MathProcessor::inputsChanged().
class SCOPY_CORE_EXPORT MathBlockWidget : public QWidget
{
	Q_OBJECT
public:
	explicit MathBlockWidget(FormulaEvaluator *evaluator, QWidget *parent = nullptr);
	explicit MathBlockWidget(MathProcessor *proc, QWidget *parent = nullptr);
};

} // namespace acq
} // namespace scopy
