#include "decoder/DecoderLogger.h"

#include "acq_engine/AcquisitionEngine.h"
#include "acq_engine/AcquisitionError.h"

namespace scopy {
namespace decoder {

DecoderLogger::DecoderLogger(QObject *parent)
	: QObject(parent)
{}

DecoderLogger::~DecoderLogger() = default;

void DecoderLogger::setEngine(scopy::acq::AcquisitionEngine *e)
{
	m_engine = e;
}

void DecoderLogger::log(LogLevel lvl, const QString &id, const QString &msg)
{
	Q_EMIT messageLogged(static_cast<int>(lvl), id, msg);

	if(!m_forward.load(std::memory_order_relaxed) || !m_engine)
		return;

	scopy::acq::AcquisitionError::Severity sev;
	switch(lvl) {
	case LogLevel::Info:     sev = scopy::acq::AcquisitionError::Severity::Info;     break;
	case LogLevel::Warning:  sev = scopy::acq::AcquisitionError::Severity::Warning;  break;
	case LogLevel::Critical: sev = scopy::acq::AcquisitionError::Severity::Critical; break;
	}
	Q_EMIT m_engine->error(static_cast<int>(sev), id, msg);
}

} // namespace decoder
} // namespace scopy
