#include "ProcessorBlock.h"

#include "AcquisitionEngine.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QWidget>

namespace scopy {
namespace acq {

ProcessorBlock::ProcessorBlock(const QString &name, QObject *parent)
	: QObject(parent)
	, m_name(name)
{}

void ProcessorBlock::setEnabled(bool en)
{
	const bool prev = m_enabled.exchange(en, std::memory_order_relaxed);
	if(prev != en)
		Q_EMIT enabledChanged(en);
}

QWidget *ProcessorBlock::createSettingsWidget(QWidget *parent)
{
	auto *w   = new QWidget(parent);
	auto *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(4);

	auto *enableCb = new QCheckBox(QStringLiteral("Enabled"), w);
	enableCb->setChecked(isEnabled());
	connect(enableCb, &QCheckBox::toggled, this, [this](bool en) { setEnabled(en); });
	connect(this, &ProcessorBlock::enabledChanged, enableCb, &QCheckBox::setChecked);
	lay->addWidget(enableCb);

	return w;
}

void ProcessorBlock::report(AcquisitionError::Severity sev, const QString &msg) const
{
	auto *engine = qobject_cast<AcquisitionEngine *>(parent());
	if(!engine)
		return;
	Q_EMIT engine->error(static_cast<int>(sev), m_name, msg);
}

} // namespace acq
} // namespace scopy
