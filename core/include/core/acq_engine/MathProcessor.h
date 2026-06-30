#pragma once

#include "scopy-core_export.h"

#include "DataKey.h"
#include "FormulaEvaluator.h"
#include "ProcessorBlock.h"

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

	void           setOutputKey(const DataKey &k) { m_outputKey = k; }
	const DataKey &outputKey() const { return m_outputKey; }

	void setWatchedKeys(const QList<DataKey> &keys) override;

	void     process(DataStore *store) override;
	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

	void              setFormula(const QString &formula);
	FormulaEvaluator &evaluator() { return m_evaluator; }

Q_SIGNALS:
	void inputsChanged();

private:
	FormulaEvaluator m_evaluator;
	DataKey          m_outputKey;
};

} // namespace acq
} // namespace scopy
