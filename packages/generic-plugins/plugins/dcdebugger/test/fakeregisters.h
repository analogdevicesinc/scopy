/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#ifndef DCDEBUGGER_TEST_FAKEREGISTERS_H
#define DCDEBUGGER_TEST_FAKEREGISTERS_H

#include <QMap>

#include <component/registerreader.h>
#include <component/registerwriter.h>
#include <qcoro/qcorotask.h>

namespace scopy::dcdebugger::test {

// A register file shared by a reader and a writer leaf; unknown addresses read as the address.
struct FakeRegisterFile
{
	QMap<uint32_t, uint32_t> regs;
	bool fail = false;
	int reads = 0;
	int writes = 0;
	uint32_t valueAt(uint32_t addr) const { return regs.value(addr, addr); }
};

class FakeRegisterReader : public component::RegisterReader
{
public:
	FakeRegisterReader(FakeRegisterFile *file, int addrBits, int valBits, QObject *parent)
		: RegisterReader(parent)
		, m_file(file)
		, m_addrBits(addrBits)
		, m_valBits(valBits)
	{}
	int addressBits() const override { return m_addrBits; }
	int valueBits() const override { return m_valBits; }
	QCoro::Task<CommandResponse<uint32_t>> readAsync(uint32_t addr) override
	{
		m_file->reads++;
		if(m_file->fail) {
			const Error e{-EIO, "fake read error"};
			Q_EMIT readFailed(e);
			co_return CommandResponse<uint32_t>(QUuid(), Unexpected<Error>{e});
		}
		const uint32_t v = m_file->valueAt(addr);
		Q_EMIT readSucceeded(addr, v);
		co_return CommandResponse<uint32_t>(QUuid(), v);
	}

private:
	FakeRegisterFile *m_file;
	int m_addrBits, m_valBits;
};

class FakeRegisterWriter : public component::RegisterWriter
{
public:
	FakeRegisterWriter(FakeRegisterFile *file, int addrBits, int valBits, QObject *parent)
		: RegisterWriter(parent)
		, m_file(file)
		, m_addrBits(addrBits)
		, m_valBits(valBits)
	{}
	int addressBits() const override { return m_addrBits; }
	int valueBits() const override { return m_valBits; }
	QCoro::Task<CommandResponse<void>> writeAsync(uint32_t addr, uint32_t value) override
	{
		m_file->writes++;
		if(m_file->fail) {
			const Error e{-EIO, "fake write error"};
			Q_EMIT writeFailed(e);
			co_return CommandResponse<void>(QUuid(), Unexpected<Error>{e});
		}
		m_file->regs[addr] = value;
		Q_EMIT writeSucceeded(addr);
		co_return CommandResponse<void>(QUuid());
	}

private:
	FakeRegisterFile *m_file;
	int m_addrBits, m_valBits;
};

} // namespace scopy::dcdebugger::test

#endif // DCDEBUGGER_TEST_FAKEREGISTERS_H
