#pragma once

#include "scopy-core_export.h"

#include "DataKey.h"
#include "FormulaEvaluator.h"
#include "SourceBlock.h"

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

	void              setFormula(const QString &formula);
	FormulaEvaluator &evaluator() { return m_evaluator; }
	const DataKey    &outputKey() const { return m_outputKey; }

private:
	FormulaEvaluator m_evaluator;
	DataKey          m_outputKey;
};

} // namespace acq
} // namespace scopy
