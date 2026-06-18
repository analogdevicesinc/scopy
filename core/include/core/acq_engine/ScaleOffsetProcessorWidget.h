#pragma once

#include "scopy-core_export.h"

#include "ScaleOffsetProcessor.h"

#include <QWidget>

namespace scopy {
namespace acq {

// GUI panel for ScaleOffsetProcessor.
// Creates one scale + offset spinbox pair per registered channel.
// Spinboxes write directly into the processor's atomic members — no extra wiring needed.
class SCOPY_CORE_EXPORT ScaleOffsetProcessorWidget : public QWidget
{
public:
	explicit ScaleOffsetProcessorWidget(ScaleOffsetProcessor *proc,
					    QWidget *parent = nullptr);
};

} // namespace acq
} // namespace scopy
