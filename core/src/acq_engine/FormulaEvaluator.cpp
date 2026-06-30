#include "FormulaEvaluator.h"

#include <cmath>

namespace scopy {
namespace acq {

const QString FormulaEvaluator::k_setupScript = QStringLiteral(
	"var sin=Math.sin,cos=Math.cos,tan=Math.tan,"
	"asin=Math.asin,acos=Math.acos,atan=Math.atan,atan2=Math.atan2,"
	"sqrt=Math.sqrt,abs=Math.abs,log=Math.log,"
	"log2=function(x){return Math.log(x)/Math.LN2;},"
	"log10=function(x){return Math.log(x)/Math.LN10;},"
	"exp=Math.exp,pow=Math.pow,"
	"floor=Math.floor,ceil=Math.ceil,"
	"min=Math.min,max=Math.max,"
	"pi=Math.PI,e=Math.E;");

FormulaEvaluator::FormulaEvaluator()
	: m_formula(QStringLiteral("sin(2*pi*S/100)"))
	, m_dirty(true)
{
	QJSEngine tempEng;
	tempEng.evaluate(k_setupScript);
	const QJSValue r = tempEng.evaluate(wrapFormula(m_formula, 0));
	m_syntaxValid.store(!r.isError(), std::memory_order_relaxed);
}

FormulaEvaluator::~FormulaEvaluator() = default;

QString FormulaEvaluator::wrapFormula(const QString &formula, int inputCount)
{
	QString xDecls;
	for(int k = 0; k < inputCount; ++k) {
		xDecls += QStringLiteral(",X%1=(xs&&xs[%2])?xs[%2][i]:0")
				  .arg(k + 1)
				  .arg(k);
	}
	return QString(
		       "(function(n,xs){"
		       "var o=new Array(n);"
		       "for(var i=0;i<n;i++){"
		       "var S=i%1;"
		       "o[i]=(%2);"
		       "}"
		       "return o;"
		       "})")
		.arg(xDecls, formula);
}

void FormulaEvaluator::setFormula(const QString &formula)
{
	int probeCount;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		probeCount = m_lastInputCount;
	}
	QJSEngine tempEng;
	tempEng.evaluate(k_setupScript);
	const QJSValue r = tempEng.evaluate(wrapFormula(formula, probeCount));
	m_syntaxValid.store(!r.isError(), std::memory_order_relaxed);

	std::lock_guard<std::mutex> lock(m_mutex);
	m_formula = formula;
	m_dirty   = true;
}

bool FormulaEvaluator::isValid() const
{
	return m_syntaxValid.load(std::memory_order_relaxed);
}

QString FormulaEvaluator::formula() const
{
	std::lock_guard<std::mutex> lock(m_mutex);
	return m_formula;
}

void FormulaEvaluator::rebuildEngine(const QString &formula, int inputCount)
{
	m_batchFn = QJSValue(); // release ref before destroying old engine
	m_workerEngine.reset(new QJSEngine());
	m_workerEngine->evaluate(k_setupScript);
	m_batchFn = m_workerEngine->evaluate(wrapFormula(formula, inputCount));
	if(m_batchFn.isError()) {
		m_batchFn = QJSValue();
		m_syntaxValid.store(false, std::memory_order_relaxed);
	}
}

bool FormulaEvaluator::evaluateBatch(int n,
				     const QVector<const QVector<float> *> &inputs,
				     QVector<float> &out,
				     QString *errorOut)
{
	QString formulaCopy;
	bool    dirty = false;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		dirty       = m_dirty || (m_lastInputCount != inputs.size());
		formulaCopy = m_formula;
		m_dirty     = false;
		m_lastInputCount = inputs.size();
	}

	if(dirty)
		rebuildEngine(formulaCopy, inputs.size());

	out.resize(n);

	if(!m_batchFn.isCallable()) {
		if(errorOut)
			*errorOut = QStringLiteral("no valid batch function");
		out.fill(0.0f);
		return false;
	}

	QJSValue jsXs = m_workerEngine->newArray(static_cast<uint>(inputs.size()));
	for(int k = 0; k < inputs.size(); ++k) {
		const QVector<float> *in = inputs[k];
		if(!in) {
			// leave slot as undefined; wrapper treats falsy as 0
			continue;
		}
		QJSValue jsArr = m_workerEngine->newArray(static_cast<uint>(n));
		const int copyN = std::min((long long)n, in->size());
		for(int i = 0; i < copyN; ++i)
			jsArr.setProperty(static_cast<quint32>(i),
					  static_cast<double>((*in)[i]));
		for(int i = copyN; i < n; ++i)
			jsArr.setProperty(static_cast<quint32>(i), 0.0);
		jsXs.setProperty(static_cast<quint32>(k), jsArr);
	}

	const QJSValue result = m_batchFn.call({QJSValue(n), jsXs});

	if(result.isError()) {
		if(errorOut)
			*errorOut = QStringLiteral("evaluation exception: %1").arg(result.toString());
		m_batchFn = QJSValue();
		m_syntaxValid.store(false, std::memory_order_relaxed);
		out.fill(0.0f);
		return false;
	}

	for(int i = 0; i < n; ++i) {
		const double v = result.property(static_cast<quint32>(i)).toNumber();
		out[i] = std::isfinite(v) ? static_cast<float>(v) : 0.0f;
	}

	m_workerEngine->collectGarbage();
	return true;
}

} // namespace acq
} // namespace scopy
