#include "ProcessorBlock.h"

#include <QWidget>

namespace scopy {
namespace acq {

ProcessorBlock::ProcessorBlock(const QString &name, QObject *parent)
	: QObject(parent)
	, m_name(name)
{}

QWidget *ProcessorBlock::createSettingsWidget(QWidget *) { return nullptr; }

} // namespace acq
} // namespace scopy
