#include "ProcessorBlock.h"

namespace scopy {
namespace acq {

ProcessorBlock::ProcessorBlock(const QString &name, QObject *parent)
	: Block(name, parent)
{}

} // namespace acq
} // namespace scopy

#include "moc_ProcessorBlock.cpp"
