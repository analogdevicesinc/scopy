#pragma once

#include <core/acq_engine/DataKey.h>
#include <core/acq_engine/SampleBuffer.h>
#include <core/acq_engine/SourceBlock.h>

#include <cmath>

namespace scopy {
namespace adc {
namespace sim {

// Generates continuous sine-wave Float32 data for each enabled channel.
class SimulatedSource : public scopy::acq::SourceBlock
{
	Q_OBJECT
	static constexpr float TWO_PI = 6.28318530f;

public:
	explicit SimulatedSource(const QString &id, QObject *parent = nullptr)
		: SourceBlock(id, parent)
	{}

	void acquire(scopy::acq::DataStore *store) override
	{
		if(m_stopRequested)
			return;

		const int count = static_cast<int>(m_bufferSize);

		int chIndex = 0;
		for(auto it = m_channels.cbegin(); it != m_channels.cend(); ++it, ++chIndex) {
			if(!it.value())
				continue;

			QVector<float> data(count);
			for(int i = 0; i < count; ++i) {
				float t = static_cast<float>(m_sampleIndex + i) / 100.0f;
				data[i] = std::sin(TWO_PI * t + chIndex * 0.5f);
			}

			store->write(scopy::acq::DataKey::raw(m_id, it.key()), std::move(data));
		}

		if (m_sampleIndex >= 10 * m_bufferSize) {
			m_sampleIndex = 0;
		} else {
			m_sampleIndex += count;
		}
	}

private:
	int m_sampleIndex = 0;
};

} // namespace sim
} // namespace adc
} // namespace scopy
