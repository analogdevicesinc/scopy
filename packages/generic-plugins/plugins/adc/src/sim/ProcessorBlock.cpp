#include "ProcessorBlock.h"

#include <QWidget>

namespace scopy {
namespace adc {
namespace sim {

ProcessorBlock::ProcessorBlock(const QString &name, QObject *parent)
	: QObject(parent)
	, m_name(name)
{}

QWidget *ProcessorBlock::createSettingsWidget(QWidget *) { return nullptr; }

} // namespace sim
} // namespace adc
} // namespace scopy
