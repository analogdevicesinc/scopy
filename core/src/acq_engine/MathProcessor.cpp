#include "MathProcessor.h"
#include "DataStore.h"
#include "MathBlockWidget.h"

#include <algorithm>
#include <deque>
#include <QWidget>

namespace scopy {
namespace acq {

MathProcessor::MathProcessor(const QString &name, QObject *parent)
	: ProcessorBlock(name, parent)
{
	m_evaluator.setFormula(QStringLiteral("X1"));
}

void MathProcessor::setWatchedKeys(const QList<DataKey> &keys)
{
	ProcessorBlock::setWatchedKeys(keys);
	Q_EMIT inputsChanged();
}

void MathProcessor::process(DataStore *store)
{
	// held owns the chunks; inputs points into it. std::deque never
	// invalidates element addresses on push_back, so the pointers stay valid.
	std::deque<QVector<float>>      held;
	QVector<const QVector<float> *> inputs;
	inputs.reserve(m_watchedKeys.size());

	int n = 0;
	for(const DataKey &k : m_watchedKeys) {
		auto src = store->latestAs<QVector<float>>(k);
		if(!src) {
			inputs.append(nullptr); // absent or non-float reads as 0
			continue;
		}
		held.push_back(std::move(*src));
		inputs.append(&held.back());
		n = std::max<int>(n, held.back().size());
	}

	if(n == 0)
		return;

	QVector<float> out(n);
	QString        err;
	if(!m_evaluator.evaluateBatch(n, inputs, out, &err))
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("formula evaluation failed (%1) — writing zeros").arg(err));
	store->write(m_outputKey, std::move(out));
}

QWidget *MathProcessor::createSettingsWidget(QWidget *parent)
{
	return withBaseSettings(new MathBlockWidget(this), parent);
}

void MathProcessor::setFormula(const QString &formula)
{
	m_evaluator.setFormula(formula);
}

} // namespace acq
} // namespace scopy
