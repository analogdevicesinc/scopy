#pragma once

#include "scopy-core_export.h"

#include "FormulaEvaluator.h"

#include <QWidget>

namespace scopy {
namespace acq {

// Shared settings widget for MathSource and MathProcessor.
// Shows a formula line-edit and a validity indicator (✓/✗).
class SCOPY_CORE_EXPORT MathBlockWidget : public QWidget
{
public:
	explicit MathBlockWidget(FormulaEvaluator *evaluator, QWidget *parent = nullptr);
};

} // namespace acq
} // namespace scopy
