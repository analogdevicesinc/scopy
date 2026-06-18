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

	bool evaluateBatch(int n, const float *xIn, QVector<float> &out); // worker thread

private:
	static QString wrapFormula(const QString &formula);
	void           rebuildEngine(const QString &formula);

	mutable std::mutex m_mutex;
	QString            m_formula;
	bool               m_dirty{true};
	std::atomic<bool>  m_syntaxValid{false};

	std::unique_ptr<QJSEngine> m_workerEngine;
	QJSValue                   m_batchFn;

	static const QString k_setupScript;
};

} // namespace acq
} // namespace scopy
