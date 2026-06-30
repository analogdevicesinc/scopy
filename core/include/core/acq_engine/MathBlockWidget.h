#pragma once

#include "scopy-core_export.h"

#include "FormulaEvaluator.h"

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
