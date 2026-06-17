#pragma once

#include "ScaleOffsetProcessor.h"

#include <QWidget>

namespace scopy {
namespace adc {

// GUI panel for ScaleOffsetProcessor.
// Creates one scale + offset spinbox pair per registered channel.
// Spinboxes write directly into the processor's atomic members — no extra wiring needed.
class ScaleOffsetProcessorWidget : public QWidget
{
public:
	explicit ScaleOffsetProcessorWidget(sim::ScaleOffsetProcessor *proc,
					    QWidget *parent = nullptr);
};

} // namespace adc
} // namespace scopy
