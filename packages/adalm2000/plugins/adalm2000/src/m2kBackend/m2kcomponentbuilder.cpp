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
 */

#include "m2kcomponentbuilder.h"

#include "m2kattributereader.h"
#include "m2kattributewriter.h"
#include "m2kcontext.h"
#include "m2kping.h"

#include "component/attribute.h"
#include "component/channel.h"
#include "component/device.h"

#include "powersupplymath.h"

#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/analog/m2kpowersupply.hpp>
#include <libm2k/m2k.hpp>

#include <QLoggingCategory>
#include <QString>

#include <memory>

Q_LOGGING_CATEGORY(CAT_M2K_BUILDER, "M2kComponentBuilder")

using namespace scopy;
using namespace scopy::adalm2000;
using namespace libm2k::analog;

namespace {

component::Device *addDevice(component::Context *ctx, const QString &id)
{
	auto *dev = new component::Device(ctx);
	dev->setId(id);
	dev->setName(id);
	dev->setLabel(id);
	return dev;
}

QByteArray rangeToLabel(M2K_RANGE r) { return r == PLUS_MINUS_2_5V ? QByteArray(PLUS_2_5V) : QByteArray(PLUS_25V); }

M2K_RANGE labelToRange(const QString &label)
{
	return label == QLatin1String(PLUS_2_5V) ? PLUS_MINUS_2_5V : PLUS_MINUS_25V;
}

void addReadOnlyAttr(component::Channel *chn, const QString &name, const QString &unit, void *resource,
		     M2kAttributeReader::Getter getter, ICmdExecutor *exec)
{
	auto *attr = new component::Attribute(chn);
	attr->setName(name);
	attr->setUnit(unit);
	attr->addReadCapability(new M2kAttributeReader(resource, std::move(getter), exec));
}

bool buildAnalogIn(component::Context *ctx, libm2k::context::M2k *m2k, ICmdExecutor *executor)
{
	component::Device *analogIn = addDevice(ctx, QStringLiteral("analog-in"));

	M2kAnalogIn *ain = nullptr;
	try {
		ain = m2k->getAnalogIn();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "getAnalogIn failed:" << e.what();
		return false;
	}
	if(!ain) {
		qWarning(CAT_M2K_BUILDER) << "getAnalogIn returned null";
		return false;
	}

	const ANALOG_IN_CHANNEL channels[] = {ANALOG_IN_CHANNEL_1, ANALOG_IN_CHANNEL_2};
	for(int i = 0; i < 2; ++i) {
		const ANALOG_IN_CHANNEL ch = channels[i];

		auto *chn = new component::Channel(analogIn);
		chn->setId(QStringLiteral("voltage%1").arg(i));
		chn->setName(QStringLiteral("Channel %1").arg(i + 1));
		chn->setLabel(chn->name());
		chn->setIsOutput(false);

		auto *range = new component::Attribute(chn);
		range->setName(QStringLiteral("range"));
		range->setOptions({QLatin1String(PLUS_25V), QLatin1String(PLUS_2_5V)});
		range->addReadCapability(new M2kAttributeReader(
			m2k, [ain, ch]() { return rangeToLabel(ain->getRange(ch)); }, executor));
		range->addWriteCapability(new M2kAttributeWriter(
			m2k, [ain, ch](const QString &v) { ain->setRange(ch, labelToRange(v)); }, executor));

		addReadOnlyAttr(
			chn, QStringLiteral("vertical_offset"), QStringLiteral("V"), m2k,
			[ain, ch]() { return QByteArray::number(ain->getVerticalOffset(ch)); }, executor);

		const unsigned int chIdx = static_cast<unsigned int>(i);
		addReadOnlyAttr(
			chn, QStringLiteral("oversampling_ratio"), QString(), m2k,
			[ain, chIdx]() { return QByteArray::number(ain->getOversamplingRatio(chIdx)); }, executor);
	}
	return true;
}

void addRail(component::Device *dev, libm2k::analog::M2kPowerSupply *supply, void *resource, int index, double minV,
	     double maxV, ICmdExecutor *exec)
{
	auto *chn = new component::Channel(dev);
	chn->setId(QStringLiteral("voltage%1").arg(index));
	chn->setName(index == 0 ? QStringLiteral("Positive output") : QStringLiteral("Negative output"));
	chn->setLabel(chn->name());
	chn->setIsOutput(true);

	const unsigned int idx = static_cast<unsigned int>(index);

	auto setpoint = std::make_shared<double>(0.0);
	auto *voltage = new component::Attribute(chn);
	voltage->setName(QStringLiteral("voltage"));
	voltage->setUnit(QStringLiteral("V"));
	voltage->setRange({minV, ps::VOLTAGE_STEP, maxV});
	voltage->addReadCapability(new M2kAttributeReader(
		resource, [setpoint]() { return QByteArray::number(*setpoint, 'f', 7); }, exec));
	voltage->addWriteCapability(new M2kAttributeWriter(
		resource,
		[supply, idx, setpoint](const QString &v) {
			const double volts = v.toDouble();
			supply->pushChannel(idx, volts);
			*setpoint = volts;
		},
		exec));

	auto enabled = std::make_shared<bool>(false);
	auto *enable = new component::Attribute(chn);
	enable->setName(QStringLiteral("enabled"));
	enable->setOptions({QStringLiteral("0"), QStringLiteral("1")});
	enable->addReadCapability(new M2kAttributeReader(
		resource, [enabled]() { return QByteArray::number(*enabled ? 1 : 0); }, exec));
	enable->addWriteCapability(new M2kAttributeWriter(
		resource,
		[supply, idx, enabled](const QString &v) {
			const bool on = (v.toInt() != 0);
			supply->enableChannel(idx, on);
			*enabled = on;
		},
		exec));

	auto *measured = new component::Attribute(chn);
	measured->setName(QStringLiteral("measured"));
	measured->setUnit(QStringLiteral("V"));
	measured->addReadCapability(new M2kAttributeReader(
		resource, [supply, idx]() { return QByteArray::number(supply->readChannel(idx), 'f', 7); }, exec));
}

void buildPowerSupply(component::Context *ctx, libm2k::context::M2k *m2k, ICmdExecutor *executor)
{
	component::Device *powerSupply = addDevice(ctx, QStringLiteral("power-supply"));

	libm2k::analog::M2kPowerSupply *supply = nullptr;
	try {
		supply = m2k->getPowerSupply();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "getPowerSupply failed:" << e.what();
	}
	if(!supply) {
		qWarning(CAT_M2K_BUILDER) << "power-supply device left empty; the Power Supply tool will be idle";
		return;
	}
	addRail(powerSupply, supply, m2k, 0, ps::POSITIVE_MIN, ps::POSITIVE_MAX, executor);
	addRail(powerSupply, supply, m2k, 1, ps::NEGATIVE_MIN, ps::NEGATIVE_MAX, executor);
}

} // namespace

bool M2kComponentBuilder::build(component::Context *ctx, ICmdExecutor *executor)
{
	auto *m2kCtx = qobject_cast<M2kContext *>(ctx);
	if(!m2kCtx || !m2kCtx->m2k() || !executor) {
		qWarning(CAT_M2K_BUILDER) << "build() called with an incomplete context";
		return false;
	}
	libm2k::context::M2k *m2k = m2kCtx->m2k();

	m2kCtx->setName(QStringLiteral("ADALM2000"));
	try {
		m2kCtx->setDescription(QStringLiteral("ADALM2000 fw %1 serial %2")
					       .arg(QString::fromStdString(m2k->getFirmwareVersion()),
						    QString::fromStdString(m2k->getSerialNumber())));
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "could not read identity:" << e.what();
		m2kCtx->setDescription(QStringLiteral("ADALM2000"));
	}

	new M2kPing(
		m2k, [m2k]() { return m2k->getLed(); }, executor, m2kCtx);

	if(!buildAnalogIn(m2kCtx, m2k, executor)) {
		return false;
	}
	addDevice(m2kCtx, QStringLiteral("analog-out"));
	buildPowerSupply(m2kCtx, m2k, executor);

	return true;
}
