#include "MathProcessor.h"
#include "DataStore.h"
#include "MathBlockWidget.h"

#include <QVBoxLayout>
#include <QWidget>
#include <algorithm>
#include <variant>

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
	const int nKeys = m_watchedKeys.size();

	// Reserve to keep addresses stable across appends.
	QVector<QVector<float>>          heldInputs;
	QVector<const QVector<float> *>  inputs;
	heldInputs.reserve(nKeys);
	inputs.reserve(nKeys);

	int n = 0;
	for(const DataKey &k : m_watchedKeys) {
		const SampleBuffer buf = store->read(k);
		if(buf.empty()) {
			heldInputs.append(QVector<float>());
			inputs.append(nullptr);
			continue;
		}
		const auto &v = buf.sample(0);
		if(!std::holds_alternative<QVector<float>>(v)) {
			heldInputs.append(QVector<float>());
			inputs.append(nullptr);
			continue;
		}
		heldInputs.append(std::get<QVector<float>>(v));
		inputs.append(&heldInputs.last());
		n = std::max((long long)n, heldInputs.last().size());
	}

	if(n == 0)
		return; // nothing to do

	QVector<float> out(n);
	QString        err;
	if(!m_evaluator.evaluateBatch(n, inputs, out, &err))
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("formula evaluation failed (%1) — writing zeros").arg(err));
	store->write(m_outputKey, std::move(out));
}

QWidget *MathProcessor::createSettingsWidget(QWidget *parent)
{
	auto *w   = new QWidget(parent);
	auto *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(4);
	lay->addWidget(ProcessorBlock::createSettingsWidget(w));
	lay->addWidget(new MathBlockWidget(this, w));
	return w;
}

void MathProcessor::setFormula(const QString &formula)
{
	m_evaluator.setFormula(formula);
}

} // namespace acq
} // namespace scopy
