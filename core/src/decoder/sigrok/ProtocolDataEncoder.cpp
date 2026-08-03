// Registry and per-PD override table for the sigrok-cli annotation-input path.

#include "ProtocolDataEncoder.h"

namespace scopy {
namespace decoder {
namespace sigrok {

void ProtocolDataEncoderRegistry::registerEncoder(std::unique_ptr<IProtocolDataEncoder> e)
{
	if(!e) return;
	m_encoders[e->downstreamId().toStdString()] = std::move(e);
}

IProtocolDataEncoder *ProtocolDataEncoderRegistry::find(const QString &downstreamId) const
{
	auto it = m_encoders.find(downstreamId.toStdString());
	return (it == m_encoders.end()) ? nullptr : it->second.get();
}

QStringList ProtocolDataEncoderRegistry::downstreamIds() const
{
	QStringList ids;
	ids.reserve(static_cast<int>(m_encoders.size()));
	for(const auto &kv : m_encoders)
		ids.append(QString::fromStdString(kv.first));
	ids.sort();
	return ids;
}

void ProtocolDataEncoderRegistry::registerBuiltins()
{
	registerEncoder(makeUartEncoder());
	registerEncoder(makeSpiEncoder());
	registerEncoder(makeI2cEncoder());
}

std::map<std::string, std::string> downstreamOptionOverrides(const QString &downstreamId)
{
	// `-I protocoldata` emits one wire per protocol, so PDs that expect split
	// TX/RX channels must be told to read both roles off it.
	static const std::map<std::string, std::map<std::string, std::string>> table = {
		{"modbus", {{"cschannel", "RX"}, {"scchannel", "RX"}}},
	};
	auto it = table.find(downstreamId.toStdString());
	return (it == table.end()) ? std::map<std::string, std::string>{} : it->second;
}

} // namespace sigrok
} // namespace decoder
} // namespace scopy
