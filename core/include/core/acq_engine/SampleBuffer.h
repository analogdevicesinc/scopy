#pragma once

#include <deque>
#include <variant>
#include <QString>
#include <QVector>

namespace scopy {
namespace acq {

struct Annotation
{
	quint64 startSample{0};
	quint64 endSample{0};
	QString decoder; // e.g. "uart-1"
	QString klass;   // annotation class / row
	QString text;
	int     severity{0};
};

enum class SampleType { Float32, Float64, Int32, Int16, Int8, UInt8, Annotation };

using SampleVariant = std::variant<
	QVector<float>,        // index 0 → SampleType::Float32
	QVector<double>,       // index 1 → SampleType::Float64
	QVector<qint32>,       // index 2 → SampleType::Int32
	QVector<qint16>,       // index 3 → SampleType::Int16
	QVector<qint8>,        // index 4 → SampleType::Int8
	QVector<quint8>,       // index 5 → SampleType::UInt8
	QVector<Annotation>>;  // index 6 → SampleType::Annotation

// SampleBuffer::type() casts variant.index() to SampleType.
// Both sequences must remain in sync.
static_assert(std::variant_size_v<SampleVariant> == 7,
	      "SampleVariant and SampleType enum must have the same number of alternatives");

struct SampleBuffer
{
	void setHistorySize(std::size_t n)
	{
		m_historySize = (n < 1) ? 1 : n;
		while(m_history.size() > m_historySize)
			m_history.pop_back();
	}
	std::size_t historySize() const { return m_historySize; }

	const SampleVariant &sample(std::size_t index = 0) const
	{
		if(index >= m_history.size())
			return m_empty;
		return m_history[index];
	}

	std::size_t depth() const { return m_history.size(); }
	bool        empty() const { return m_history.empty(); }

	SampleType type() const noexcept
	{
		if(m_history.empty())
			return SampleType::Float32;
		return static_cast<SampleType>(m_history.front().index());
	}

	std::size_t size() const
	{
		if(m_history.empty()) return 0;
		return std::visit([](const auto &v) -> std::size_t {
			return static_cast<std::size_t>(v.size());
		}, m_history.front());
	}

	void push(SampleVariant vec)
	{
		m_history.push_front(std::move(vec));
		while(m_history.size() > m_historySize)
			m_history.pop_back();
	}

private:
	std::size_t               m_historySize{1};
	std::deque<SampleVariant> m_history;
	static const SampleVariant m_empty;
};

inline const SampleVariant SampleBuffer::m_empty = QVector<float>{};

} // namespace acq
} // namespace scopy
