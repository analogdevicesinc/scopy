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
	: m_formula(QStringLiteral("sin(2*pi*T/100)"))
	, m_dirty(true)
{
	QJSEngine tempEng;
	tempEng.evaluate(k_setupScript);
	const QJSValue r = tempEng.evaluate(wrapFormula(m_formula));
	m_syntaxValid.store(!r.isError(), std::memory_order_relaxed);
}

FormulaEvaluator::~FormulaEvaluator() = default;

QString FormulaEvaluator::wrapFormula(const QString &formula)
{
	return QString(
		       "(function(n,xs){"
		       "var o=new Array(n);"
		       "for(var i=0;i<n;i++){"
		       "var T=i,X=xs?xs[i]:0;"
		       "o[i]=(%1);"
		       "}"
		       "return o;"
		       "})")
		.arg(formula);
}

void FormulaEvaluator::setFormula(const QString &formula)
{
	QJSEngine tempEng;
	tempEng.evaluate(k_setupScript);
	const QJSValue r = tempEng.evaluate(wrapFormula(formula));
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

void FormulaEvaluator::rebuildEngine(const QString &formula)
{
	m_batchFn = QJSValue(); // release ref before destroying old engine
	m_workerEngine.reset(new QJSEngine());
	m_workerEngine->evaluate(k_setupScript);
	m_batchFn = m_workerEngine->evaluate(wrapFormula(formula));
	if(m_batchFn.isError()) {
		m_batchFn = QJSValue();
		m_syntaxValid.store(false, std::memory_order_relaxed);
	}
}

bool FormulaEvaluator::evaluateBatch(int n, const float *xIn, QVector<float> &out)
{
	QString formulaCopy;
	bool    dirty = false;
	{
		std::lock_guard<std::mutex> lock(m_mutex);
		dirty       = m_dirty;
		formulaCopy = m_formula;
		m_dirty     = false;
	}

	if(dirty)
		rebuildEngine(formulaCopy);

	out.resize(n);

	if(!m_batchFn.isCallable()) {
		qWarning("[FormulaEvaluator] no valid batch function — writing zeros");
		out.fill(0.0f);
		return false;
	}

	QJSValue jsXs;
	if(xIn) {
		jsXs = m_workerEngine->newArray(static_cast<uint>(n));
		for(int i = 0; i < n; ++i)
			jsXs.setProperty(static_cast<quint32>(i), static_cast<double>(xIn[i]));
	}
	// default-constructed QJSValue is undefined — falsy in JS, so X falls back to 0

	const QJSValue result = m_batchFn.call({QJSValue(n), jsXs});

	if(result.isError()) {
		qWarning("[FormulaEvaluator] evaluation exception: %s",
			 qPrintable(result.toString()));
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
