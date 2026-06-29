#include "ProcessorBlock.h"

#include "AcquisitionEngine.h"

#include <QWidget>

namespace scopy {
namespace acq {

ProcessorBlock::ProcessorBlock(const QString &name, QObject *parent)
	: QObject(parent)
	, m_name(name)
{}

QWidget *ProcessorBlock::createSettingsWidget(QWidget *) { return nullptr; }

void ProcessorBlock::report(AcquisitionError::Severity sev, const QString &msg) const
{
	auto *engine = qobject_cast<AcquisitionEngine *>(parent());
	if(!engine)
		return;
	Q_EMIT engine->error(static_cast<int>(sev), m_name, msg);
}

} // namespace acq
} // namespace scopy
