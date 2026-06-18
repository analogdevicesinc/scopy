#include "MathProcessor.h"
#include "DataStore.h"
#include "MathBlockWidget.h"

#include <variant>

namespace scopy {
namespace acq {

MathProcessor::MathProcessor(const QString &name, QObject *parent)
	: ProcessorBlock(name, parent)
{
	m_evaluator.setFormula(QStringLiteral("X"));
}

void MathProcessor::configure(const DataKey &inputKey, const DataKey &outputKey)
{
	m_inputKey    = inputKey;
	m_outputKey   = outputKey;
	m_watchedKeys = {inputKey};
}

void MathProcessor::process(DataStore *store)
{
	const SampleBuffer buf = store->read(m_inputKey);
	if(buf.empty())
		return;

	const auto &v = buf.sample(0);
	if(!std::holds_alternative<QVector<float>>(v))
		return;

	const QVector<float> &in = std::get<QVector<float>>(v);
	QVector<float>        out(in.size());
	if(!m_evaluator.evaluateBatch(in.size(), in.constData(), out))
		qWarning("[MathProcessor] formula evaluation failed — writing zeros for '%s'",
			 qPrintable(m_name));
	store->write(m_outputKey, std::move(out));
}

QWidget *MathProcessor::createSettingsWidget(QWidget *parent)
{
	return new MathBlockWidget(&m_evaluator, parent);
}

void MathProcessor::setFormula(const QString &formula)
{
	m_evaluator.setFormula(formula);
}

} // namespace acq
} // namespace scopy
