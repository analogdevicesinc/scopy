#include "SampleBuffer.h"

#include <algorithm>
#include <type_traits>

namespace scopy {
namespace acq {

namespace {

const SampleVariant &emptyChunk()
{
	static const SampleVariant e = QVector<float>{};
	return e;
}

// Empty chunk of a given type, so reads of a known-but-unwritten stream don't
// masquerade as Float32.
SampleVariant emptyChunkOf(std::optional<SampleType> t)
{
	switch(t.value_or(SampleType::Float32)) {
	case SampleType::Float64:    return QVector<double>{};
	case SampleType::Int32:      return QVector<qint32>{};
	case SampleType::Int16:      return QVector<qint16>{};
	case SampleType::Int8:       return QVector<qint8>{};
	case SampleType::UInt8:      return QVector<quint8>{};
	case SampleType::Annotation: return QVector<Annotation>{};
	case SampleType::Float32:    break;
	}
	return QVector<float>{};
}

// True for the one alternative that isn't a numeric sample vector.
template<class VecT>
constexpr bool isAnnotationVec = std::is_same_v<VecT, QVector<Annotation>>;

} // namespace

QString sampleTypeName(SampleType t)
{
	switch(t) {
	case SampleType::Float32:    return QStringLiteral("f32");
	case SampleType::Float64:    return QStringLiteral("f64");
	case SampleType::Int32:      return QStringLiteral("i32");
	case SampleType::Int16:      return QStringLiteral("i16");
	case SampleType::Int8:       return QStringLiteral("i8");
	case SampleType::UInt8:      return QStringLiteral("u8");
	case SampleType::Annotation: return QStringLiteral("ann");
	}
	return QStringLiteral("?");
}

const char *textRadixName(TextRadix r)
{
	switch(r) {
	case TextRadix::Hex:   return "hex";
	case TextRadix::Dec:   return "dec";
	case TextRadix::Oct:   return "oct";
	case TextRadix::Bin:   return "bin";
	case TextRadix::Ascii: return "ascii";
	}
	return "?";
}

const char *reprKindName(ReprKind k)
{
	switch(k) {
	case ReprKind::Hidden:      return "hidden";
	case ReprKind::Curve:       return "curve";
	case ReprKind::Digital:     return "digital";
	case ReprKind::Annotations: return "annotations";
	case ReprKind::Waterfall:   return "waterfall";
	}
	return "?";
}

bool textRadixFromString(const QString &s, TextRadix &out)
{
	const QString r = s.trimmed().toLower();
	if(r == QStringLiteral("hex")   || r == QStringLiteral("16")) { out = TextRadix::Hex;   return true; }
	if(r == QStringLiteral("dec")   || r == QStringLiteral("10")) { out = TextRadix::Dec;   return true; }
	if(r == QStringLiteral("oct")   || r == QStringLiteral("8"))  { out = TextRadix::Oct;   return true; }
	if(r == QStringLiteral("bin")   || r == QStringLiteral("2"))  { out = TextRadix::Bin;   return true; }
	if(r == QStringLiteral("ascii") || r == QStringLiteral("char")) { out = TextRadix::Ascii; return true; }
	return false;
}

QVector<float> toFloat(const SampleVariant &v)
{
	return std::visit([](const auto &vec) -> QVector<float> {
		using VecT = std::decay_t<decltype(vec)>;
		if constexpr(isAnnotationVec<VecT>) {
			return {};
		} else if constexpr(std::is_same_v<VecT, QVector<float>>) {
			return vec;
		} else {
			QVector<float> out(vec.size());
			std::copy(vec.cbegin(), vec.cend(), out.begin());
			return out;
		}
	}, v);
}

FloatView toFloatView(const SampleVariant &v, QVector<float> &scratch)
{
	return std::visit([&](const auto &vec) -> FloatView {
		using VecT = std::decay_t<decltype(vec)>;
		if constexpr(isAnnotationVec<VecT>) {
			return {};
		} else if constexpr(std::is_same_v<VecT, QVector<float>>) {
			return {vec.constData(), static_cast<int>(vec.size())};
		} else {
			scratch.resize(vec.size());
			std::copy(vec.cbegin(), vec.cend(), scratch.begin());
			return {scratch.constData(), static_cast<int>(scratch.size())};
		}
	}, v);
}

QVector<quint8> toBits(const SampleVariant &v)
{
	return std::visit([](const auto &vec) -> QVector<quint8> {
		using VecT = std::decay_t<decltype(vec)>;
		if constexpr(isAnnotationVec<VecT>) {
			return {};
		} else {
			QVector<quint8> out(vec.size());
			std::transform(vec.cbegin(), vec.cend(), out.begin(),
				       [](auto s) -> quint8 { return s != 0 ? 1 : 0; });
			return out;
		}
	}, v);
}

SampleVariant truncateWindow(const SampleVariant &v, int n)
{
	if(n <= 0)
		return emptyChunkOf(sampleTypeOf(v));

	return std::visit([&](const auto &vec) -> SampleVariant {
		using VecT = std::decay_t<decltype(vec)>;
		if constexpr(isAnnotationVec<VecT>)
			return vec;
		else
			return vec.size() > n ? vec.mid(0, n) : vec;
	}, v);
}

SampleVariant shiftAnnotations(const SampleVariant &v, int n, int delta)
{
	if(n <= 0)
		return emptyChunkOf(sampleTypeOf(v));

	return std::visit([&](const auto &vec) -> SampleVariant {
		using VecT = std::decay_t<decltype(vec)>;
		if constexpr(!isAnnotationVec<VecT>) {
			return vec;
		} else {
			if(delta == 0)
				return vec;
			VecT out;
			out.reserve(vec.size());
			for(Annotation a : vec) {
				// Signed throughout: offsets are unsigned, so a
				// negative delta must not wrap them.
				const qint64 lo = static_cast<qint64>(a.startSample) + delta;
				const qint64 hi = static_cast<qint64>(a.endSample)   + delta;
				if(hi < 0 || lo >= n)
					continue;
				a.startSample = static_cast<quint64>(std::max<qint64>(lo, 0));
				a.endSample   = static_cast<quint64>(std::min<qint64>(hi, n - 1));
				out.append(a);
			}
			return out;
		}
	}, v);
}

void SampleBuffer::setCapacity(std::size_t n)
{
	m_capacity = std::max<std::size_t>(1, n);
	while(m_history.size() > m_capacity)
		m_history.pop_back();
}

void SampleBuffer::push(SampleVariant vec)
{
	m_type = sampleTypeOf(vec);
	m_history.push_front(std::move(vec));
	while(m_history.size() > m_capacity)
		m_history.pop_back();
}

const SampleVariant &SampleBuffer::sample(std::size_t index) const
{
	return index < m_history.size() ? m_history[index] : emptyChunk();
}

std::size_t SampleBuffer::size() const
{
	if(m_history.empty())
		return 0;
	return std::visit([](const auto &v) { return static_cast<std::size_t>(v.size()); },
			  m_history.front());
}

SampleVariant SampleBuffer::window(int plotSize) const
{
	if(m_history.empty() || plotSize <= 0)
		return emptyChunkOf(m_type);

	return std::visit([&](const auto &newest) -> SampleVariant {
		using VecT = std::decay_t<decltype(newest)>;
		if constexpr(isAnnotationVec<VecT>) {
			// Annotations carry absolute sample offsets, so the newest
			// chunk already is the window.
			return newest;
		} else {
			if(newest.size() >= plotSize)
				return newest.mid(newest.size() - plotSize);

			// Walk back through history until the window is covered,
			// then copy those chunks oldest-first.
			std::size_t used = 0, have = 0;
			while(used < m_history.size() && have < static_cast<std::size_t>(plotSize))
				have += std::get<VecT>(m_history[used++]).size();

			VecT out;
			out.reserve(static_cast<int>(have));
			for(std::size_t i = used; i-- > 0;)
				out.append(std::get<VecT>(m_history[i]));
			if(out.size() > plotSize)
				out = out.mid(out.size() - plotSize);
			return out;
		}
	}, m_history.front());
}

} // namespace acq
} // namespace scopy
