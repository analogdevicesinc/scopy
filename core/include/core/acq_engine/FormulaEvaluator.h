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

	void    setFormula(const QString &formula); // GUI thread
	bool    isValid() const;                    // GUI thread
	QString formula() const;                    // GUI thread

	// Worker thread. Inputs are exposed in JS as X1, X2, ..., XN (N = inputs.size()).
	// The sample index is available as S. A null pointer in `inputs` is treated
	// as a 0-valued input. On failure returns false, fills `out` with zeros and
	// writes a human-readable reason into `errorOut` (if non-null).
	bool evaluateBatch(int n,
			   const QVector<const QVector<float> *> &inputs,
			   QVector<float> &out,
			   QString *errorOut = nullptr);

private:
	static QString wrapFormula(const QString &formula, int inputCount);
	void           rebuildEngine(const QString &formula, int inputCount);

	mutable std::mutex m_mutex;
	QString            m_formula;
	bool               m_dirty{true};
	std::atomic<bool>  m_syntaxValid{false};
	int                m_lastInputCount{0};

	std::unique_ptr<QJSEngine> m_workerEngine;
	QJSValue                   m_batchFn;

	static const QString k_setupScript;
};

} // namespace acq
} // namespace scopy
