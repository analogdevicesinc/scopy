#include "Block.h"

#include "AcquisitionEngine.h"

#include <QCheckBox>
#include <QVBoxLayout>
#include <QWidget>

namespace scopy {
namespace acq {

Block::Block(const QString &name, QObject *parent)
	: QObject(parent)
	, m_name(name)
{}

void Block::setEnabled(bool en)
{
	if(m_enabled.exchange(en, std::memory_order_relaxed) != en)
		Q_EMIT enabledChanged(en);
}

void Block::report(AcquisitionError::Severity sev, const QString &msg) const
{
	auto *engine = qobject_cast<AcquisitionEngine *>(parent());
	if(!engine || sev < engine->minReportSeverity())
		return;
	Q_EMIT engine->error(static_cast<int>(sev), m_name, msg);
}

bool Block::wantsReport(AcquisitionError::Severity sev) const
{
	auto *engine = qobject_cast<AcquisitionEngine *>(parent());
	return engine && sev >= engine->minReportSeverity();
}

QWidget *Block::createSettingsWidget(QWidget *parent)
{
	auto *cb = new QCheckBox(QStringLiteral("Enabled"), parent);
	cb->setChecked(isEnabled());
	connect(cb, &QCheckBox::toggled, this, [this](bool en) { setEnabled(en); });
	connect(this, &Block::enabledChanged, cb, &QCheckBox::setChecked);
	return cb;
}

QWidget *Block::withBaseSettings(QWidget *own, QWidget *parent)
{
	auto *w   = new QWidget(parent);
	auto *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(4);
	lay->addWidget(Block::createSettingsWidget(w));
	if(own) {
		own->setParent(w);
		lay->addWidget(own);
	}
	return w;
}

} // namespace acq
} // namespace scopy
