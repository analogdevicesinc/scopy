#include "MathSource.h"
#include "DataStore.h"
#include "MathBlockWidget.h"

#include <QVBoxLayout>
#include <QWidget>

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
	const QVector<const QVector<float> *> noInputs;
	if(!m_evaluator.evaluateBatch(out.size(), noInputs, out, &err))
		report(AcquisitionError::Severity::Warning,
		       QStringLiteral("formula evaluation failed (%1) — writing zeros").arg(err));
	store->write(m_outputKey, std::move(out));
}

QWidget *MathSource::createSettingsWidget(QWidget *parent)
{
	auto *w   = new QWidget(parent);
	auto *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(4);
	lay->addWidget(SourceBlock::createSettingsWidget(w));
	lay->addWidget(new MathBlockWidget(&m_evaluator, w));
	return w;
}

void MathSource::setFormula(const QString &formula)
{
	m_evaluator.setFormula(formula);
}

} // namespace acq
} // namespace scopy
