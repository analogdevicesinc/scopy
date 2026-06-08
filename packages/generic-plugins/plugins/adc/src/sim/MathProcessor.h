#pragma once

#include "DataKey.h"
#include "FormulaEvaluator.h"
#include "ProcessorBlock.h"

namespace scopy {
namespace adc {
namespace sim {

// ProcessorBlock that transforms an input channel by evaluating a user-supplied formula.
// Variables: T = sample index (0 .. N-1), X = input sample value.
// Output key is set via configure().
class MathProcessor : public ProcessorBlock
{
	Q_OBJECT
public:
	explicit MathProcessor(const QString &name, QObject *parent = nullptr);
	~MathProcessor() override = default;

	void configure(const DataKey &inputKey, const DataKey &outputKey);

	void     process(DataStore *store) override;
	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

	void              setFormula(const QString &formula);
	FormulaEvaluator &evaluator() { return m_evaluator; }

private:
	FormulaEvaluator m_evaluator;
	DataKey          m_inputKey;
	DataKey          m_outputKey;
};

} // namespace sim
} // namespace adc
} // namespace scopy
