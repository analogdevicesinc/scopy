#pragma once

#include "FormulaEvaluator.h"

#include <QWidget>

namespace scopy {
namespace adc {

// Shared settings widget for MathSource and MathProcessor.
// Shows a formula line-edit and a validity indicator (✓/✗).
class MathBlockWidget : public QWidget
{
public:
	explicit MathBlockWidget(sim::FormulaEvaluator *evaluator, QWidget *parent = nullptr);
};

} // namespace adc
} // namespace scopy
