/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "wavfilereader.h"

#include <algorithm>
#include <cstring>

#include <QFile>
#include <QFileInfo>
#include <QtEndian>

namespace scopy {
namespace adc {

namespace {

constexpr quint16 FORMAT_PCM = 0x0001;
constexpr quint16 FORMAT_FLOAT = 0x0003;
constexpr quint16 FORMAT_EXTENSIBLE = 0xFFFE;
constexpr quint16 MAX_CHANNELS = 256;
constexpr qint64 SLICE_FRAMES = 1 << 16;

float decodeSample(const char *p, int bytes, bool isFloat)
{
	const uchar *u = reinterpret_cast<const uchar *>(p);

	if(isFloat) {
		if(bytes == 4) {
			float v;
			const quint32 raw = qFromLittleEndian<quint32>(u);
			std::memcpy(&v, &raw, sizeof(v));
			return v;
		}
		double v;
		const quint64 raw = qFromLittleEndian<quint64>(u);
		std::memcpy(&v, &raw, sizeof(v));
		return float(v);
	}

	switch(bytes) {
	case 1:
		return (float(u[0]) - 128.0f) / 128.0f;
	case 2:
		return float(qFromLittleEndian<qint16>(u)) / 32768.0f;
	case 3: {
		qint32 v = qint32(u[0]) | (qint32(u[1]) << 8) | (qint32(u[2]) << 16);
		if(v & 0x00800000) {
			v |= ~0x00FFFFFF;
		}
		return float(v) / 8388608.0f;
	}
	default:
		return float(qFromLittleEndian<qint32>(u)) / 2147483648.0f;
	}
}

QStringList channelNames(int count)
{
	if(count == 1) {
		return {QStringLiteral("mono")};
	}
	if(count == 2) {
		return {QStringLiteral("left"), QStringLiteral("right")};
	}
	QStringList names;
	for(int i = 0; i < count; ++i) {
		names.append(QStringLiteral("ch%1").arg(i));
	}
	return names;
}

} // namespace

bool WavFileReader::open(const QString &path, QString *err)
{
	const QString name = QFileInfo(path).fileName();
	const auto fail = [err](const QString &reason) {
		if(err) {
			*err = reason;
		}
		return false;
	};

	QFile f(path);
	if(!f.open(QIODevice::ReadOnly)) {
		return fail(QStringLiteral("Cannot open %1: %2").arg(path, f.errorString()));
	}

	m_data.clear();
	m_names.clear();
	m_sampleRate = 0.0;
	m_bits = 0;

	char riff[12];
	if(f.read(riff, 12) != 12 || std::memcmp(riff, "RIFF", 4) != 0 || std::memcmp(riff + 8, "WAVE", 4) != 0) {
		return fail(QStringLiteral("%1 is not a RIFF/WAVE file").arg(name));
	}

	quint16 format = 0;
	quint16 channels = 0;
	quint16 bits = 0;
	quint32 rate = 0;
	bool haveFmt = false;
	qint64 dataPos = -1;
	qint64 dataSize = 0;

	char hdr[8];
	while(f.read(hdr, 8) == 8) {
		const quint32 size = qFromLittleEndian<quint32>(reinterpret_cast<const uchar *>(hdr + 4));
		const qint64 body = f.pos();
		const qint64 next = body + size + (size & 1);

		if(std::memcmp(hdr, "fmt ", 4) == 0 && size >= 16) {
			char fmt[16];
			if(f.read(fmt, 16) != 16) {
				return fail(QStringLiteral("%1 has a truncated fmt chunk").arg(name));
			}
			const uchar *p = reinterpret_cast<const uchar *>(fmt);
			format = qFromLittleEndian<quint16>(p);
			channels = qFromLittleEndian<quint16>(p + 2);
			rate = qFromLittleEndian<quint32>(p + 4);
			bits = qFromLittleEndian<quint16>(p + 14);
			haveFmt = true;

			if(format == FORMAT_EXTENSIBLE && size >= 40) {
				char guid[2];
				if(!f.seek(body + 24) || f.read(guid, 2) != 2) {
					return fail(QStringLiteral("%1 has a truncated fmt chunk").arg(name));
				}
				format = qFromLittleEndian<quint16>(reinterpret_cast<const uchar *>(guid));
			}
		} else if(std::memcmp(hdr, "data", 4) == 0) {
			dataPos = body;
			dataSize = size > 0 ? std::min<qint64>(size, f.size() - body) : f.size() - body;
		}

		if(!f.seek(next)) {
			break;
		}
	}

	if(!haveFmt) {
		return fail(QStringLiteral("%1 has no fmt chunk").arg(name));
	}
	if(dataPos < 0) {
		return fail(QStringLiteral("%1 has no data chunk").arg(name));
	}

	const bool isFloat = format == FORMAT_FLOAT;
	if(format != FORMAT_PCM && !isFloat) {
		return fail(QStringLiteral("%1 uses compressed format 0x%2 — only uncompressed PCM and IEEE float "
					   "WAV are supported")
				    .arg(name)
				    .arg(format, 4, 16, QChar('0')));
	}
	if(isFloat ? (bits != 32 && bits != 64) : (bits != 8 && bits != 16 && bits != 24 && bits != 32)) {
		return fail(QStringLiteral("%1 has an unsupported sample width of %2 bits").arg(name).arg(bits));
	}
	if(channels == 0 || channels > MAX_CHANNELS) {
		return fail(QStringLiteral("%1 declares %2 channels").arg(name).arg(channels));
	}

	const int sampleBytes = bits / 8;
	const qint64 frameBytes = qint64(sampleBytes) * channels;
	const qint64 frames = dataSize / frameBytes;
	if(frames <= 0) {
		return fail(QStringLiteral("%1 contains no samples").arg(name));
	}
	if(!f.seek(dataPos)) {
		return fail(QStringLiteral("Cannot read the data chunk of %1").arg(name));
	}

	for(int c = 0; c < channels; ++c) {
		m_data.append(QVector<float>());
		m_data.last().reserve(frames);
	}

	for(qint64 done = 0; done < frames;) {
		const QByteArray slice = f.read(std::min(SLICE_FRAMES, frames - done) * frameBytes);
		const qint64 got = slice.size() / frameBytes;
		if(got <= 0) {
			break;
		}
		const char *p = slice.constData();
		for(qint64 i = 0; i < got; ++i) {
			for(int c = 0; c < channels; ++c, p += sampleBytes) {
				m_data[c].append(decodeSample(p, sampleBytes, isFloat));
			}
		}
		done += got;
	}

	if(m_data.first().isEmpty()) {
		return fail(QStringLiteral("Cannot read the data chunk of %1").arg(name));
	}

	m_names = channelNames(channels);
	m_sampleRate = rate;
	m_bits = bits;
	return true;
}

const QVector<float> &WavFileReader::samples(int index) const
{
	if(index < 0 || index >= m_data.size()) {
		return emptyChannel();
	}
	return m_data.at(index);
}

QString WavFileReader::unit(int index) const
{
	Q_UNUSED(index)
	return QStringLiteral("FS");
}

} // namespace adc
} // namespace scopy
