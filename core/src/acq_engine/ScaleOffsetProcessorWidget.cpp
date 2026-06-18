#include "ScaleOffsetProcessorWidget.h"

#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QVBoxLayout>

namespace scopy {
namespace acq {

ScaleOffsetProcessorWidget::ScaleOffsetProcessorWidget(ScaleOffsetProcessor *proc,
						       QWidget *parent)
	: QWidget(parent)
{
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(8, 8, 8, 8);
	lay->setSpacing(4);

	bool firstChannel = true;
	for(ScaleOffsetProcessor::ChannelConfig *cfg : proc->channels()) {
		if(!firstChannel) {
			auto *sep = new QFrame(this);
			sep->setFrameShape(QFrame::HLine);
			sep->setFrameShadow(QFrame::Sunken);
			lay->addWidget(sep);
		}
		firstChannel = false;

		auto *chLabel = new QLabel(cfg->label, this);
		QFont f = chLabel->font();
		f.setBold(true);
		chLabel->setFont(f);
		lay->addWidget(chLabel);

		auto *form = new QFormLayout();
		form->setContentsMargins(0, 0, 0, 0);
		form->setSpacing(4);

		auto *scaleSpin = new QDoubleSpinBox(this);
		scaleSpin->setRange(-1000.0, 1000.0);
		scaleSpin->setDecimals(3);
		scaleSpin->setSingleStep(0.1);
		scaleSpin->setValue(cfg->scale.load(std::memory_order_relaxed));
		form->addRow("Scale:", scaleSpin);

		auto *offsetSpin = new QDoubleSpinBox(this);
		offsetSpin->setRange(-1e6, 1e6);
		offsetSpin->setDecimals(3);
		offsetSpin->setSingleStep(1.0);
		offsetSpin->setValue(cfg->offset.load(std::memory_order_relaxed));
		form->addRow("Offset:", offsetSpin);

		lay->addLayout(form);

		connect(scaleSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
			[cfg](double v) { cfg->scale.store(static_cast<float>(v), std::memory_order_relaxed); });

		connect(offsetSpin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
			[cfg](double v) { cfg->offset.store(static_cast<float>(v), std::memory_order_relaxed); });
	}

	lay->addStretch();
}

} // namespace acq
} // namespace scopy
