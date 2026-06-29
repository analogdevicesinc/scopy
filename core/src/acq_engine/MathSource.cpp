#include "MathSource.h"
#include "DataStore.h"
#include "MathBlockWidget.h"

namespace scopy {
namespace acq {

MathSource::MathSource(const QString &id, QObject *parent)
	: SourceBlock(id, parent)
	, m_outputKey(DataKey::withStage(id, "out", "math"))
{}

void MathSource::acquire(DataStore *store)
{
	QVector<float> out(static_cast<int>(m_bufferSize));
	QString        err;
	if(!m_evaluator.evaluateBatch(out.size(), nullptr, out, &err))
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("formula evaluation failed (%1) — writing zeros").arg(err));
	store->write(m_outputKey, std::move(out));
}

QWidget *MathSource::createSettingsWidget(QWidget *parent)
{
	return new MathBlockWidget(&m_evaluator, parent);
}

void MathSource::setFormula(const QString &formula)
{
	m_evaluator.setFormula(formula);
}

} // namespace acq
} // namespace scopy
