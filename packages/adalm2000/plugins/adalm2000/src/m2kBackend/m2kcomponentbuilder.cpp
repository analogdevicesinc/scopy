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
#include "m2kdigitalstreams.h"
#include "m2kinputstream.h"
#include "m2koutputstream.h"
#include "m2ktrace.h"

#include "component/attribute.h"
#include "component/channel.h"
#include "component/device.h"
#include "component/tags.h"

#include "digitaliomath.h"
#include "m2ktriggermath.h"
#include "powersupplymath.h"

#include <libm2k/analog/dmm.hpp>
#include <libm2k/analog/m2kanalogin.hpp>
#include <libm2k/analog/m2kanalogout.hpp>
#include <libm2k/analog/m2kpowersupply.hpp>
#include <libm2k/digital/m2kdigital.hpp>
#include <libm2k/m2k.hpp>
#include <libm2k/m2khardwaretrigger.hpp>

#include <QLoggingCategory>
#include <QPointer>
#include <QSet>
#include <QString>

#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

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

component::Attribute *addAttr(QObject *owner, const QString &name, const QString &unit)
{
	auto *attr = new component::Attribute(owner);
	attr->setName(name);
	attr->setUnit(unit);
	return attr;
}

void addReadOnlyAttr(QObject *owner, const QString &name, const QString &unit, void *resource,
		     M2kAttributeReader::Getter getter, ICmdExecutor *exec)
{
	addAttr(owner, name, unit)->addReadCapability(new M2kAttributeReader(resource, std::move(getter), exec));
}

void addWriteOnlyAttr(QObject *owner, const QString &name, const QString &unit, void *resource,
		      M2kAttributeWriter::Setter setter, ICmdExecutor *exec)
{
	addAttr(owner, name, unit)->addWriteCapability(new M2kAttributeWriter(resource, std::move(setter), exec));
}

component::Attribute *addReadWriteAttr(QObject *owner, const QString &name, const QString &unit, void *resource,
				       M2kAttributeReader::Getter getter, M2kAttributeWriter::Setter setter,
				       ICmdExecutor *exec)
{
	component::Attribute *attr = addAttr(owner, name, unit);
	attr->addReadCapability(new M2kAttributeReader(resource, std::move(getter), exec));
	attr->addWriteCapability(new M2kAttributeWriter(resource, std::move(setter), exec));
	return attr;
}

QByteArray boundsToBytes(const std::pair<double, double> &bounds)
{
	return QByteArray::number(bounds.first, 'f', 6) + ":" + QByteArray::number(bounds.second, 'f', 6);
}

QByteArray ratesToBytes(const std::vector<double> &rates)
{
	QByteArray out;
	for(size_t i = 0; i < rates.size(); ++i) {
		if(i) {
			out += ",";
		}
		out += QByteArray::number(rates[i], 'g', 12);
	}
	return out;
}

QByteArray rangesToBytes(const std::vector<std::pair<std::string, std::pair<double, double>>> &ranges)
{
	QByteArray out;
	for(size_t i = 0; i < ranges.size(); ++i) {
		if(i) {
			out += ",";
		}
		out += QByteArray::fromStdString(ranges[i].first) + "[" + boundsToBytes(ranges[i].second) + "]";
	}
	return out;
}

QStringList ratesToOptions(const std::vector<double> &rates)
{
	QStringList out;
	for(double r : rates) {
		out.append(QString::number(r, 'g', 12));
	}
	return out;
}

using libm2k::M2kHardwareTrigger;

static_assert(trigger::optionCount(trigger::MODE_OPTIONS) == libm2k::N_DIGITAL_XOR_ANALOG + 1);
static_assert(trigger::optionCount(trigger::ANALOG_CONDITION_OPTIONS) == libm2k::HIGH_LEVEL_ANALOG + 1);
static_assert(trigger::optionCount(trigger::DIGITAL_CONDITION_OPTIONS) == libm2k::NO_TRIGGER_DIGITAL + 1);
static_assert(trigger::DIGITAL_CONDITION_NONE == libm2k::NO_TRIGGER_DIGITAL);
static_assert(trigger::optionCount(trigger::ANALOG_SOURCE_OPTIONS) == libm2k::NO_SOURCE + 1);
static_assert(trigger::optionCount(trigger::DIGITAL_SOURCE_OPTIONS) == libm2k::SRC_DISABLED + 1);
static_assert(trigger::optionCount(trigger::OUT_SELECT_OPTIONS) == libm2k::SELECT_DIGITAL_IN + 1);
static_assert(trigger::optionCount(trigger::DIGITAL_MODE_OPTIONS) == libm2k::digital::DIO_AND + 1);

template <typename Enum, size_t N>
Enum enumFromName(const char *const (&table)[N], const QString &name)
{
	const int value = trigger::fromName(table, name);
	if(value < 0) {
		throw std::invalid_argument("unknown trigger option: " + name.toStdString());
	}
	return static_cast<Enum>(value);
}

template <size_t N>
QByteArray nameBytes(const char *const (&table)[N], int value)
{
	return trigger::toName(table, value).toLatin1();
}

M2kHardwareTrigger *triggerOf(M2kAnalogIn *ain)
{
	try {
		return ain->getTrigger();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "M2kAnalogIn::getTrigger failed:" << e.what();
	}
	return nullptr;
}

M2kHardwareTrigger *triggerOf(libm2k::digital::M2kDigital *dig)
{
	try {
		return dig->getTrigger();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "M2kDigital::getTrigger failed:" << e.what();
	}
	return nullptr;
}

struct TriggerCaps
{
	bool externalIn = false;
	bool externalOut = false;
	bool crossInstrument = false;
};

TriggerCaps probeTriggerCaps(M2kHardwareTrigger *trig)
{
	TriggerCaps caps;
	try {
		caps.externalIn = trig->hasExternalTriggerIn();
		caps.externalOut = trig->hasExternalTriggerOut();
		caps.crossInstrument = trig->hasCrossInstrumentTrigger();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "trigger capability probe failed:" << e.what();
	}
	return caps;
}

void addAnalogTriggerChannelAttrs(component::Channel *chn, M2kHardwareTrigger *trig, void *resource, unsigned int chIdx,
				  ICmdExecutor *exec)
{
	using namespace trigger;

	component::Attribute *mode = addReadWriteAttr(
		chn, QStringLiteral("trigger_mode"), QString(), resource,
		[trig, chIdx]() {
			return nameBytes(MODE_OPTIONS,
					 M2K_CALL(trig, "M2kHardwareTrigger::getAnalogMode", trig->getAnalogMode(chIdx),
						  M2K_ARG(chIdx)));
		},
		[trig, chIdx](const QString &v) {
			const auto mode = enumFromName<libm2k::M2K_TRIGGER_MODE>(MODE_OPTIONS, v);
			M2K_CALL(trig, "M2kHardwareTrigger::setAnalogMode", trig->setAnalogMode(chIdx, mode),
				 M2K_ARG(chIdx), M2K_ARG(mode));
		},
		exec);
	mode->setOptions(toOptions(MODE_OPTIONS));

	addReadWriteAttr(
		chn, QStringLiteral("trigger_level"), QStringLiteral("V"), resource,
		[trig, chIdx]() {
			return QByteArray::number(M2K_CALL(trig, "M2kHardwareTrigger::getAnalogLevel",
							   trig->getAnalogLevel(chIdx), M2K_ARG(chIdx)),
						  'g', 12);
		},
		[trig, chIdx](const QString &v) {
			const double v_level = v.toDouble();
			M2K_CALL(trig, "M2kHardwareTrigger::setAnalogLevel", trig->setAnalogLevel(chIdx, v_level),
				 M2K_ARG(chIdx), M2K_ARG(v_level));
		},
		exec);

	addReadWriteAttr(
		chn, QStringLiteral("trigger_hysteresis"), QStringLiteral("V"), resource,
		[trig, chIdx]() {
			return QByteArray::number(M2K_CALL(trig, "M2kHardwareTrigger::getAnalogHysteresis",
							   trig->getAnalogHysteresis(chIdx), M2K_ARG(chIdx)),
						  'g', 12);
		},
		[trig, chIdx](const QString &v) {
			const double hysteresis = v.toDouble();
			M2K_CALL(trig, "M2kHardwareTrigger::setAnalogHysteresis",
				 trig->setAnalogHysteresis(chIdx, hysteresis), M2K_ARG(chIdx), M2K_ARG(hysteresis));
		},
		exec);

	component::Attribute *condition = addReadWriteAttr(
		chn, QStringLiteral("trigger_condition"), QString(), resource,
		[trig, chIdx]() {
			return nameBytes(ANALOG_CONDITION_OPTIONS,
					 M2K_CALL(trig, "M2kHardwareTrigger::getAnalogCondition",
						  trig->getAnalogCondition(chIdx), M2K_ARG(chIdx)));
		},
		[trig, chIdx](const QString &v) {
			const auto cond =
				enumFromName<libm2k::M2K_TRIGGER_CONDITION_ANALOG>(ANALOG_CONDITION_OPTIONS, v);
			M2K_CALL(trig, "M2kHardwareTrigger::setAnalogCondition", trig->setAnalogCondition(chIdx, cond),
				 M2K_ARG(chIdx), M2K_ARG(cond));
		},
		exec);
	condition->setOptions(toOptions(ANALOG_CONDITION_OPTIONS));

	component::Attribute *externalCondition = addReadWriteAttr(
		chn, QStringLiteral("trigger_external_condition"), QString(), resource,
		[trig, chIdx]() {
			return nameBytes(DIGITAL_CONDITION_OPTIONS,
					 M2K_CALL(trig, "M2kHardwareTrigger::getAnalogExternalCondition",
						  trig->getAnalogExternalCondition(chIdx), M2K_ARG(chIdx)));
		},
		[trig, chIdx](const QString &v) {
			const auto cond =
				enumFromName<libm2k::M2K_TRIGGER_CONDITION_DIGITAL>(DIGITAL_CONDITION_OPTIONS, v);
			M2K_CALL(trig, "M2kHardwareTrigger::setAnalogExternalCondition",
				 trig->setAnalogExternalCondition(chIdx, cond), M2K_ARG(chIdx), M2K_ARG(cond));
		},
		exec);
	externalCondition->setOptions(toOptions(DIGITAL_CONDITION_OPTIONS, DIGITAL_CONDITION_NONE));
}

void addAnalogTriggerDeviceAttrs(component::Device *dev, M2kHardwareTrigger *trig, const TriggerCaps &caps,
				 void *resource, ICmdExecutor *exec)
{
	using namespace trigger;

	component::Attribute *source = addReadWriteAttr(
		dev, QStringLiteral("trigger_source"), QString(), resource,
		[trig]() {
			return nameBytes(
				ANALOG_SOURCE_OPTIONS,
				M2K_CALL(trig, "M2kHardwareTrigger::getAnalogSource", trig->getAnalogSource()));
		},
		[trig](const QString &v) {
			const auto src = enumFromName<libm2k::M2K_TRIGGER_SOURCE_ANALOG>(ANALOG_SOURCE_OPTIONS, v);
			M2K_CALL(trig, "M2kHardwareTrigger::setAnalogSource", trig->setAnalogSource(src), M2K_ARG(src));
		},
		exec);
	source->setOptions(caps.crossInstrument ? toOptions(ANALOG_SOURCE_OPTIONS)
						: toOptions(ANALOG_SOURCE_OPTIONS, libm2k::SRC_DIGITAL_IN));

	component::Attribute *delay = addReadWriteAttr(
		dev, QStringLiteral("trigger_delay"), QStringLiteral("samples"), resource,
		[trig]() {
			return QByteArray::number(
				M2K_CALL(trig, "M2kHardwareTrigger::getAnalogDelay", trig->getAnalogDelay()));
		},
		[trig](const QString &v) {
			const int delay = v.toInt();
			M2K_CALL(trig, "M2kHardwareTrigger::setAnalogDelay", trig->setAnalogDelay(delay),
				 M2K_ARG(delay));
		},
		exec);
	delay->setRange({static_cast<double>(DELAY_MIN), 1.0, static_cast<double>(-DELAY_MIN)});

	component::Attribute *streaming = addReadWriteAttr(
		dev, QStringLiteral("trigger_streaming_flag"), QString(), resource,
		[trig]() {
			return QByteArray::number(M2K_CALL(trig, "M2kHardwareTrigger::getAnalogStreamingFlag",
							   trig->getAnalogStreamingFlag())
							  ? 1
							  : 0);
		},
		[trig](const QString &v) {
			const bool enable = v.toInt() != 0;
			M2K_CALL(trig, "M2kHardwareTrigger::setAnalogStreamingFlag",
				 trig->setAnalogStreamingFlag(enable), M2K_ARG(enable));
		},
		exec);
	streaming->setOptions({QStringLiteral("0"), QStringLiteral("1")});

	if(caps.externalOut) {
		component::Attribute *outSelect = addReadWriteAttr(
			dev, QStringLiteral("trigger_external_out_select"), QString(), resource,
			[trig]() {
				return nameBytes(OUT_SELECT_OPTIONS,
						 M2K_CALL(trig, "M2kHardwareTrigger::getAnalogExternalOutSelect",
							  trig->getAnalogExternalOutSelect()));
			},
			[trig](const QString &v) {
				const auto output_select =
					enumFromName<libm2k::M2K_TRIGGER_OUT_SELECT>(OUT_SELECT_OPTIONS, v);
				M2K_CALL(trig, "M2kHardwareTrigger::setAnalogExternalOutSelect",
					 trig->setAnalogExternalOutSelect(output_select), M2K_ARG(output_select));
			},
			exec);
		outSelect->setOptions(toOptions(OUT_SELECT_OPTIONS));
	}

	addReadOnlyAttr(
		dev, QStringLiteral("has_external_trigger_in"), QString(), resource,
		[trig]() {
			return QByteArray::number(
				M2K_CALL(trig, "M2kHardwareTrigger::hasExternalTriggerIn", trig->hasExternalTriggerIn())
					? 1
					: 0);
		},
		exec);
	addReadOnlyAttr(
		dev, QStringLiteral("has_external_trigger_out"), QString(), resource,
		[trig]() {
			return QByteArray::number(M2K_CALL(trig, "M2kHardwareTrigger::hasExternalTriggerOut",
							   trig->hasExternalTriggerOut())
							  ? 1
							  : 0);
		},
		exec);
	addReadOnlyAttr(
		dev, QStringLiteral("has_cross_instrument_trigger"), QString(), resource,
		[trig]() {
			return QByteArray::number(M2K_CALL(trig, "M2kHardwareTrigger::hasCrossInstrumentTrigger",
							   trig->hasCrossInstrumentTrigger())
							  ? 1
							  : 0);
		},
		exec);
}

component::Channel *buildAnalogInChannel(component::Device *analogIn, M2kAnalogIn *ain, void *resource, int index,
					 ICmdExecutor *executor)
{
	const ANALOG_IN_CHANNEL ch = (index == 0) ? ANALOG_IN_CHANNEL_1 : ANALOG_IN_CHANNEL_2;
	const unsigned int chIdx = static_cast<unsigned int>(index);

	auto *chn = new component::Channel(analogIn);
	chn->setId(QStringLiteral("voltage%1").arg(index));
	chn->setName(QStringLiteral("Channel %1").arg(index + 1));
	chn->setLabel(chn->name());
	chn->setIsOutput(false);
	chn->setStreamIndex(index);

	component::Attribute *range = addReadWriteAttr(
		chn, QStringLiteral("range"), QString(), resource,
		[ain, ch]() {
			return rangeToLabel(M2K_CALL(ain, "M2kAnalogIn::getRange", ain->getRange(ch), M2K_ARG(ch)));
		},
		[ain, ch](const QString &v) {
			const M2K_RANGE range = labelToRange(v);
			M2K_CALL(ain, "M2kAnalogIn::setRange", ain->setRange(ch, range), M2K_ARG(ch), M2K_ARG(range));
		},
		executor);
	range->setOptions({QLatin1String(PLUS_25V), QLatin1String(PLUS_2_5V)});

	addReadWriteAttr(
		chn, QStringLiteral("vertical_offset"), QStringLiteral("V"), resource,
		[ain, ch]() {
			return QByteArray::number(M2K_CALL(ain, "M2kAnalogIn::getVerticalOffset",
							   ain->getVerticalOffset(ch), M2K_ARG(ch)));
		},
		[ain, ch](const QString &v) {
			const double vertOffset = v.toDouble();
			M2K_CALL(ain, "M2kAnalogIn::setVerticalOffset", ain->setVerticalOffset(ch, vertOffset),
				 M2K_ARG(ch), M2K_ARG(vertOffset));
		},
		executor);

	addReadOnlyAttr(
		chn, QStringLiteral("range_limits"), QStringLiteral("V"), resource,
		[ain, ch]() {
			const M2K_RANGE range = M2K_CALL(ain, "M2kAnalogIn::getRange", ain->getRange(ch), M2K_ARG(ch));
			return boundsToBytes(M2K_CALL(ain, "M2kAnalogIn::getRangeLimits", ain->getRangeLimits(range),
						      M2K_ARG(range)));
		},
		executor);

	addReadOnlyAttr(
		chn, QStringLiteral("hysteresis_range"), QStringLiteral("V"), resource,
		[ain, ch]() {
			return boundsToBytes(M2K_CALL(ain, "M2kAnalogIn::getHysteresisRange",
						      ain->getHysteresisRange(ch), M2K_ARG(ch)));
		},
		executor);

	addReadOnlyAttr(
		chn, QStringLiteral("scaling_factor"), QString(), resource,
		[ain, ch]() {
			return QByteArray::number(
				M2K_CALL(ain, "M2kAnalogIn::getScalingFactor", ain->getScalingFactor(ch), M2K_ARG(ch)),
				'g', 12);
		},
		executor);

	addReadOnlyAttr(
		chn, QStringLiteral("range_value"), QString(), resource,
		[ain, ch]() {
			const M2K_RANGE range = M2K_CALL(ain, "M2kAnalogIn::getRange", ain->getRange(ch), M2K_ARG(ch));
			return QByteArray::number(M2K_CALL(ain, "M2kAnalogIn::getValueForRange",
							   ain->getValueForRange(range), M2K_ARG(range)),
						  'g', 12);
		},
		executor);

	addReadOnlyAttr(
		chn, QStringLiteral("enabled"), QString(), resource,
		[ain, chIdx]() {
			return QByteArray::number(M2K_CALL(ain, "M2kAnalogIn::isChannelEnabled",
							   ain->isChannelEnabled(chIdx), M2K_ARG(chIdx))
							  ? 1
							  : 0);
		},
		executor);
	return chn;
}

void buildAnalogInDeviceAttrs(component::Device *analogIn, M2kAnalogIn *ain, void *resource, ICmdExecutor *executor)
{
	addReadOnlyAttr(
		analogIn, QStringLiteral("name"), QString(), resource,
		[ain]() { return QByteArray::fromStdString(M2K_CALL(ain, "M2kAnalogIn::getName", ain->getName())); },
		executor);

	addReadOnlyAttr(
		analogIn, QStringLiteral("nb_channels"), QString(), resource,
		[ain]() {
			return QByteArray::number(M2K_CALL(ain, "M2kAnalogIn::getNbChannels", ain->getNbChannels()));
		},
		executor);

	addReadOnlyAttr(
		analogIn, QStringLiteral("maximum_samplerate"), QStringLiteral("Hz"), resource,
		[ain]() {
			return QByteArray::number(
				M2K_CALL(ain, "M2kAnalogIn::getMaximumSamplerate", ain->getMaximumSamplerate()), 'g',
				12);
		},
		executor);

	addReadOnlyAttr(
		analogIn, QStringLiteral("available_sample_rates"), QStringLiteral("Hz"), resource,
		[ain]() {
			return ratesToBytes(
				M2K_CALL(ain, "M2kAnalogIn::getAvailableSampleRates", ain->getAvailableSampleRates()));
		},
		executor);

	addReadOnlyAttr(
		analogIn, QStringLiteral("available_ranges"), QStringLiteral("V"), resource,
		[ain]() {
			return rangesToBytes(
				M2K_CALL(ain, "M2kAnalogIn::getAvailableRanges", ain->getAvailableRanges()));
		},
		executor);

	addReadOnlyAttr(
		analogIn, QStringLiteral("filter_compensation"), QString(), resource,
		[ain]() {
			const double samplerate = M2K_CALL(ain, "M2kAnalogIn::getSampleRate", ain->getSampleRate());
			return QByteArray::number(M2K_CALL(ain, "M2kAnalogIn::getFilterCompensation",
							   ain->getFilterCompensation(samplerate), M2K_ARG(samplerate)),
						  'g', 12);
		},
		executor);

	addReadOnlyAttr(
		analogIn, QStringLiteral("low_range_max"), QStringLiteral("V"), resource,
		[ain]() {
			return QByteArray::number(M2K_CALL(ain, "M2kAnalogIn::getRangeLimits",
							   ain->getRangeLimits(PLUS_MINUS_2_5V),
							   M2K_ARGN("range", PLUS_MINUS_2_5V))
							  .second,
						  'f', 6);
		},
		executor);

	component::Attribute *sampleRate = addReadWriteAttr(
		analogIn, QStringLiteral("sample_rate"), QStringLiteral("Hz"), resource,
		[ain]() {
			return QByteArray::number(M2K_CALL(ain, "M2kAnalogIn::getSampleRate", ain->getSampleRate()),
						  'g', 12);
		},
		[ain](const QString &v) {
			const double samplerate = v.toDouble();
			M2K_CALL(ain, "M2kAnalogIn::setSampleRate", ain->setSampleRate(samplerate),
				 M2K_ARG(samplerate));
		},
		executor);
	// The acquisition view times its samples with it (found by tag, like the IIO sampling_frequency).
	component::addTag(sampleRate, component::tags::SampleRate);
	try {
		sampleRate->setOptions(ratesToOptions(ain->getAvailableSampleRates()));
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "getAvailableSampleRates failed:" << e.what();
	}

	addReadWriteAttr(
		analogIn, QStringLiteral("oversampling_ratio"), QString(), resource,
		[ain]() {
			return QByteArray::number(
				M2K_CALL(ain, "M2kAnalogIn::getOversamplingRatio", ain->getOversamplingRatio()));
		},
		[ain](const QString &v) {
			const int oversampling = v.toInt();
			M2K_CALL(ain, "M2kAnalogIn::setOversamplingRatio", ain->setOversamplingRatio(oversampling),
				 M2K_ARG(oversampling));
		},
		executor);
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

	M2kHardwareTrigger *trig = triggerOf(ain);
	QList<component::Channel *> members;
	for(int i = 0; i < 2; ++i) {
		component::Channel *chn = buildAnalogInChannel(analogIn, ain, m2k, i, executor);
		members.append(chn);
		if(trig) {
			addAnalogTriggerChannelAttrs(chn, trig, m2k, static_cast<unsigned int>(i), executor);
		}
	}
	buildAnalogInDeviceAttrs(analogIn, ain, m2k, executor);
	if(trig) {
		addAnalogTriggerDeviceAttrs(analogIn, trig, probeTriggerCaps(trig), m2k, executor);
	}

	auto *in = new M2kInputStream(m2k, ain, executor, analogIn);
	for(component::Channel *chn : members) {
		in->addChannel(chn);
	}
	return true;
}

component::Channel *buildAnalogOutChannel(component::Device *analogOut, M2kAnalogOut *aout, void *resource, int index,
					  ICmdExecutor *executor)
{
	const unsigned int chIdx = static_cast<unsigned int>(index);

	auto *chn = new component::Channel(analogOut);
	chn->setId(QStringLiteral("voltage%1").arg(index));
	try {
		chn->setName(QString::fromStdString(aout->getChannelName(chIdx)));
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "getChannelName failed:" << e.what();
		chn->setName(QStringLiteral("DAC %1").arg(index + 1));
	}
	chn->setLabel(chn->name());
	chn->setIsOutput(true);
	chn->setStreamIndex(index);

	component::Attribute *enabled = addReadWriteAttr(
		chn, QStringLiteral("enabled"), QString(), resource,
		[aout, chIdx]() {
			return QByteArray::number(M2K_CALL(aout, "M2kAnalogOut::isChannelEnabled",
							   aout->isChannelEnabled(chIdx), M2K_ARG(chIdx))
							  ? 1
							  : 0);
		},
		[aout, chIdx](const QString &v) {
			const bool enable = v.toInt() != 0;
			M2K_CALL(aout, "M2kAnalogOut::enableChannel", aout->enableChannel(chIdx, enable),
				 M2K_ARG(chIdx), M2K_ARG(enable));
		},
		executor);
	enabled->setOptions({QStringLiteral("0"), QStringLiteral("1")});

	component::Attribute *sampleRate = addReadWriteAttr(
		chn, QStringLiteral("sample_rate"), QStringLiteral("Hz"), resource,
		[aout, chIdx]() {
			return QByteArray::number(M2K_CALL(aout, "M2kAnalogOut::getSampleRate",
							   aout->getSampleRate(chIdx), M2K_ARG(chIdx)),
						  'g', 12);
		},
		[aout, chIdx](const QString &v) {
			const double samplerate = v.toDouble();
			M2K_CALL(aout, "M2kAnalogOut::setSampleRate", aout->setSampleRate(chIdx, samplerate),
				 M2K_ARG(chIdx), M2K_ARG(samplerate));
		},
		executor);
	// The generator computes its waveforms for this rate (found by tag on the output channels).
	component::addTag(sampleRate, component::tags::SampleRate);
	try {
		sampleRate->setOptions(ratesToOptions(aout->getAvailableSampleRates(chIdx)));
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "getAvailableSampleRates failed:" << e.what();
	}

	addReadWriteAttr(
		chn, QStringLiteral("oversampling_ratio"), QString(), resource,
		[aout, chIdx]() {
			return QByteArray::number(M2K_CALL(aout, "M2kAnalogOut::getOversamplingRatio",
							   aout->getOversamplingRatio(chIdx), M2K_ARG(chIdx)));
		},
		[aout, chIdx](const QString &v) {
			const int oversampling = v.toInt();
			M2K_CALL(aout, "M2kAnalogOut::setOversamplingRatio",
				 aout->setOversamplingRatio(chIdx, oversampling), M2K_ARG(chIdx),
				 M2K_ARG(oversampling));
		},
		executor);

	component::Attribute *cyclic = addReadWriteAttr(
		chn, QStringLiteral("cyclic"), QString(), resource,
		[aout, chIdx]() {
			return QByteArray::number(
				M2K_CALL(aout, "M2kAnalogOut::getCyclic", aout->getCyclic(chIdx), M2K_ARG(chIdx)) ? 1
														  : 0);
		},
		[aout, chIdx](const QString &v) {
			const bool en = v.toInt() != 0;
			M2K_CALL(aout, "M2kAnalogOut::setCyclic", aout->setCyclic(chIdx, en), M2K_ARG(chIdx),
				 M2K_ARG(en));
		},
		executor);
	cyclic->setOptions({QStringLiteral("0"), QStringLiteral("1")});

	addReadWriteAttr(
		chn, QStringLiteral("kernel_buffers_count"), QString(), resource,
		[aout, chIdx]() {
			return QByteArray::number(M2K_CALL(aout, "M2kAnalogOut::getKernelBuffersCount",
							   aout->getKernelBuffersCount(chIdx), M2K_ARG(chIdx)));
		},
		[aout, chIdx](const QString &v) {
			const unsigned int count = static_cast<unsigned int>(v.toUInt());
			M2K_CALL(aout, "M2kAnalogOut::setKernelBuffersCount", aout->setKernelBuffersCount(chIdx, count),
				 M2K_ARG(chIdx), M2K_ARG(count));
		},
		executor);

	addReadOnlyAttr(
		chn, QStringLiteral("maximum_samplerate"), QStringLiteral("Hz"), resource,
		[aout, chIdx]() {
			return QByteArray::number(M2K_CALL(aout, "M2kAnalogOut::getMaximumSamplerate",
							   aout->getMaximumSamplerate(chIdx), M2K_ARG(chIdx)),
						  'g', 12);
		},
		executor);

	addReadOnlyAttr(
		chn, QStringLiteral("available_sample_rates"), QStringLiteral("Hz"), resource,
		[aout, chIdx]() {
			return ratesToBytes(M2K_CALL(aout, "M2kAnalogOut::getAvailableSampleRates",
						     aout->getAvailableSampleRates(chIdx), M2K_ARG(chIdx)));
		},
		executor);

	addReadOnlyAttr(
		chn, QStringLiteral("scaling_factor"), QString(), resource,
		[aout, chIdx]() {
			return QByteArray::number(M2K_CALL(aout, "M2kAnalogOut::getScalingFactor",
							   aout->getScalingFactor(chIdx), M2K_ARG(chIdx)),
						  'g', 12);
		},
		executor);

	addReadOnlyAttr(
		chn, QStringLiteral("filter_compensation"), QString(), resource,
		[aout, chIdx]() {
			const double samplerate = M2K_CALL(aout, "M2kAnalogOut::getSampleRate",
							   aout->getSampleRate(chIdx), M2K_ARG(chIdx));
			return QByteArray::number(M2K_CALL(aout, "M2kAnalogOut::getFilterCompensation",
							   aout->getFilterCompensation(samplerate),
							   M2K_ARG(samplerate)),
						  'g', 12);
		},
		executor);

#ifndef LIBIIO_V1
	addReadOnlyAttr(
		chn, QStringLiteral("push_done"), QString(), resource,
		[aout, chIdx]() {
			return QByteArray::number(
				M2K_CALL(aout, "M2kAnalogOut::isPushDone", aout->isPushDone(chIdx), M2K_ARG(chIdx))
					? 1
					: 0);
		},
		executor);
#endif

	addWriteOnlyAttr(
		chn, QStringLiteral("voltage"), QStringLiteral("V"), resource,
		[aout, chIdx](const QString &v) {
			const double volts = v.toDouble();
			M2K_CALL(aout, "M2kAnalogOut::setVoltage", aout->setVoltage(chIdx, volts), M2K_ARG(chIdx),
				 M2K_ARG(volts));
		},
		executor);
	return chn;
}

void buildAnalogOut(component::Context *ctx, libm2k::context::M2k *m2k, ICmdExecutor *executor)
{
	component::Device *analogOut = addDevice(ctx, QStringLiteral("analog-out"));

	M2kAnalogOut *aout = nullptr;
	try {
		aout = m2k->getAnalogOut();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "getAnalogOut failed:" << e.what();
	}
	if(!aout) {
		qWarning(CAT_M2K_BUILDER) << "analog-out device left empty; DAC tools will be idle";
		return;
	}

	unsigned int channels = 2;
	try {
		channels = aout->getNbChannels();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "getNbChannels failed, assuming 2:" << e.what();
	}

	addReadOnlyAttr(
		analogOut, QStringLiteral("nb_channels"), QString(), m2k,
		[aout]() {
			return QByteArray::number(M2K_CALL(aout, "M2kAnalogOut::getNbChannels", aout->getNbChannels()));
		},
		executor);

	QList<component::Channel *> members;
	for(unsigned int i = 0; i < channels; ++i) {
		members.append(buildAnalogOutChannel(analogOut, aout, m2k, static_cast<int>(i), executor));
	}

	auto *out = new M2kOutputStream(m2k, aout, executor, analogOut);
	for(component::Channel *chn : members) {
		out->addChannel(chn);
	}
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
			M2K_CALL(supply, "M2kPowerSupply::pushChannel", supply->pushChannel(idx, volts), M2K_ARG(idx),
				 M2K_ARG(volts));
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
			M2K_CALL(supply, "M2kPowerSupply::enableChannel", supply->enableChannel(idx, on), M2K_ARG(idx),
				 M2K_ARG(on));
			*enabled = on;
		},
		exec));

	auto *measured = new component::Attribute(chn);
	measured->setName(QStringLiteral("measured"));
	measured->setUnit(QStringLiteral("V"));
	measured->addReadCapability(new M2kAttributeReader(
		resource,
		[supply, idx]() {
			return QByteArray::number(
				M2K_CALL(supply, "M2kPowerSupply::readChannel", supply->readChannel(idx), M2K_ARG(idx)),
				'f', 7);
		},
		exec));
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

using libm2k::digital::DIO_CHANNEL;
using libm2k::digital::DIO_DIRECTION;
using libm2k::digital::DIO_LEVEL;
using libm2k::digital::DIO_MODE;
using libm2k::digital::M2kDigital;

using DioStatePtr = std::shared_ptr<dio::PinState>;

void writePinDirection(M2kDigital *dig, const dio::PinState &state, int pin)
{
	const bool output = state.outputEnabled && dio::bit(state.direction, pin);
	const DIO_CHANNEL index = static_cast<DIO_CHANNEL>(pin);
	const DIO_DIRECTION dir = output ? DIO_DIRECTION::DIO_OUTPUT : DIO_DIRECTION::DIO_INPUT;
	M2K_CALL(dig, "M2kDigital::setDirection", dig->setDirection(index, dir), M2K_ARG(index), M2K_ARG(dir));
}

void writePinValue(M2kDigital *dig, const dio::PinState &state, int pin)
{
	if(!state.outputEnabled) {
		return;
	}
	const DIO_CHANNEL index = static_cast<DIO_CHANNEL>(pin);
	const DIO_LEVEL level = dio::bit(state.gpo, pin) ? DIO_LEVEL::HIGH : DIO_LEVEL::LOW;
	M2K_CALL(dig, "M2kDigital::setValueRaw", dig->setValueRaw(index, level), M2K_ARG(index), M2K_ARG(level));
}

void addDigitalTriggerPinAttr(component::Channel *chn, M2kHardwareTrigger *trig, void *resource, int pin,
			      ICmdExecutor *exec)
{
	using namespace trigger;
	const DIO_CHANNEL dioPin = static_cast<DIO_CHANNEL>(pin);

	component::Attribute *condition = addReadWriteAttr(
		chn, QStringLiteral("trigger_condition"), QString(), resource,
		[trig, dioPin]() {
			return nameBytes(DIGITAL_CONDITION_OPTIONS,
					 M2K_CALL(trig, "M2kHardwareTrigger::getDigitalCondition",
						  trig->getDigitalCondition(dioPin), M2K_ARG(dioPin)));
		},
		[trig, dioPin](const QString &v) {
			const auto cond =
				enumFromName<libm2k::M2K_TRIGGER_CONDITION_DIGITAL>(DIGITAL_CONDITION_OPTIONS, v);
			M2K_CALL(trig, "M2kHardwareTrigger::setDigitalCondition",
				 trig->setDigitalCondition(dioPin, cond), M2K_ARG(dioPin), M2K_ARG(cond));
		},
		exec);
	condition->setOptions(toOptions(DIGITAL_CONDITION_OPTIONS));
}

void addDigitalTriggerDeviceAttrs(component::Device *dev, M2kHardwareTrigger *trig, const TriggerCaps &caps,
				  void *resource, ICmdExecutor *exec)
{
	using namespace trigger;

	component::Attribute *mode = addReadWriteAttr(
		dev, QStringLiteral("trigger_mode"), QString(), resource,
		[trig]() {
			return nameBytes(DIGITAL_MODE_OPTIONS,
					 M2K_CALL(trig, "M2kHardwareTrigger::getDigitalMode", trig->getDigitalMode()));
		},
		[trig](const QString &v) {
			const auto mode = enumFromName<libm2k::digital::DIO_TRIGGER_MODE>(DIGITAL_MODE_OPTIONS, v);
			M2K_CALL(trig, "M2kHardwareTrigger::setDigitalMode", trig->setDigitalMode(mode), M2K_ARG(mode));
		},
		exec);
	mode->setOptions(toOptions(DIGITAL_MODE_OPTIONS));

	component::Attribute *delay = addReadWriteAttr(
		dev, QStringLiteral("trigger_delay"), QStringLiteral("samples"), resource,
		[trig]() {
			return QByteArray::number(
				M2K_CALL(trig, "M2kHardwareTrigger::getDigitalDelay", trig->getDigitalDelay()));
		},
		[trig](const QString &v) {
			const int delay = v.toInt();
			M2K_CALL(trig, "M2kHardwareTrigger::setDigitalDelay", trig->setDigitalDelay(delay),
				 M2K_ARG(delay));
		},
		exec);
	delay->setRange({static_cast<double>(DELAY_MIN), 1.0, static_cast<double>(-DELAY_MIN)});

	try {
		trig->setDigitalStreamingFlag(false);
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "setDigitalStreamingFlag(false) failed:" << e.what();
	}
	component::Attribute *streaming = addReadWriteAttr(
		dev, QStringLiteral("trigger_streaming_flag"), QString(), resource,
		[trig]() {
			return QByteArray::number(M2K_CALL(trig, "M2kHardwareTrigger::getDigitalStreamingFlag",
							   trig->getDigitalStreamingFlag())
							  ? 1
							  : 0);
		},
		[trig](const QString &v) {
			const bool enable = v.toInt() != 0;
			M2K_CALL(trig, "M2kHardwareTrigger::setDigitalStreamingFlag",
				 trig->setDigitalStreamingFlag(enable), M2K_ARG(enable));
		},
		exec);
	streaming->setOptions({QStringLiteral("0"), QStringLiteral("1")});

	if(caps.crossInstrument) {
		component::Attribute *source = addReadWriteAttr(
			dev, QStringLiteral("trigger_source"), QString(), resource,
			[trig]() {
				return nameBytes(DIGITAL_SOURCE_OPTIONS,
						 M2K_CALL(trig, "M2kHardwareTrigger::getDigitalSource",
							  trig->getDigitalSource()));
			},
			[trig](const QString &v) {
				const auto external_src =
					enumFromName<libm2k::M2K_TRIGGER_SOURCE_DIGITAL>(DIGITAL_SOURCE_OPTIONS, v);
				M2K_CALL(trig, "M2kHardwareTrigger::setDigitalSource",
					 trig->setDigitalSource(external_src), M2K_ARG(external_src));
			},
			exec);
		source->setOptions(toOptions(DIGITAL_SOURCE_OPTIONS));
	}

	component::Attribute *externalCondition = addReadWriteAttr(
		dev, QStringLiteral("trigger_external_condition"), QString(), resource,
		[trig]() {
			return nameBytes(DIGITAL_CONDITION_OPTIONS,
					 M2K_CALL(trig, "M2kHardwareTrigger::getDigitalExternalCondition",
						  trig->getDigitalExternalCondition()));
		},
		[trig](const QString &v) {
			const auto cond =
				enumFromName<libm2k::M2K_TRIGGER_CONDITION_DIGITAL>(DIGITAL_CONDITION_OPTIONS, v);
			M2K_CALL(trig, "M2kHardwareTrigger::setDigitalExternalCondition",
				 trig->setDigitalExternalCondition(cond), M2K_ARG(cond));
		},
		exec);
	externalCondition->setOptions(toOptions(DIGITAL_CONDITION_OPTIONS));
}

component::Channel *addPin(component::Device *dev, M2kDigital *dig, void *resource, int pin, DioStatePtr state,
			   ICmdExecutor *exec)
{
	auto *chn = new component::Channel(dev);
	chn->setId(QStringLiteral("voltage%1").arg(pin));
	chn->setName(QStringLiteral("DIO %1").arg(pin));
	chn->setLabel(chn->name());
	chn->setIsOutput(false);
	chn->setStreamIndex(pin);

	auto *direction = new component::Attribute(chn);
	direction->setName(QStringLiteral("direction"));
	direction->setOptions({QLatin1String(dio::DIRECTION_OPTIONS[0]), QLatin1String(dio::DIRECTION_OPTIONS[1])});
	direction->addReadCapability(new M2kAttributeReader(
		resource, [state, pin]() { return QByteArray(dio::bit(state->direction, pin) ? "out" : "in"); }, exec));
	direction->addWriteCapability(new M2kAttributeWriter(
		resource,
		[dig, state, pin](const QString &v) {
			state->direction = dio::withBit(state->direction, pin, v == QLatin1String("out"));
			writePinDirection(dig, *state, pin);
		},
		exec));

	auto *raw = new component::Attribute(chn);
	raw->setName(QStringLiteral("raw"));
	raw->setOptions({QLatin1String(dio::VALUE_OPTIONS[0]), QLatin1String(dio::VALUE_OPTIONS[1])});
	raw->addReadCapability(new M2kAttributeReader(
		resource, [state, pin]() { return QByteArray::number(dio::bit(state->gpo, pin) ? 1 : 0); }, exec));
	raw->addWriteCapability(new M2kAttributeWriter(
		resource,
		[dig, state, pin](const QString &v) {
			state->gpo = dio::withBit(state->gpo, pin, v.toInt() != 0);
			writePinValue(dig, *state, pin);
		},
		exec));

	component::Attribute *outputMode = addReadWriteAttr(
		chn, QStringLiteral("output_mode"), QString(), resource,
		[dig, pin]() {
			const DIO_CHANNEL chn = static_cast<DIO_CHANNEL>(pin);
			return QByteArray(M2K_CALL(dig, "M2kDigital::getOutputMode", dig->getOutputMode(chn),
						   M2K_ARG(chn)) == DIO_MODE::DIO_OPENDRAIN
						  ? dio::OUTPUT_MODE_OPTIONS[1]
						  : dio::OUTPUT_MODE_OPTIONS[0]);
		},
		[dig, pin](const QString &v) {
			const DIO_CHANNEL chn = static_cast<DIO_CHANNEL>(pin);
			const DIO_MODE mode = v == QLatin1String(dio::OUTPUT_MODE_OPTIONS[1]) ? DIO_MODE::DIO_OPENDRAIN
											      : DIO_MODE::DIO_PUSHPULL;
			M2K_CALL(dig, "M2kDigital::setOutputMode", dig->setOutputMode(chn, mode), M2K_ARG(chn),
				 M2K_ARG(mode));
		},
		exec);
	outputMode->setOptions(
		{QLatin1String(dio::OUTPUT_MODE_OPTIONS[0]), QLatin1String(dio::OUTPUT_MODE_OPTIONS[1])});

	addReadOnlyAttr(
		chn, QStringLiteral("direction_hw"), QString(), resource,
		[dig, pin]() {
			const DIO_CHANNEL index = static_cast<DIO_CHANNEL>(pin);
			return QByteArray(M2K_CALL(dig, "M2kDigital::getDirection", dig->getDirection(index),
						   M2K_ARG(index)) == DIO_DIRECTION::DIO_OUTPUT
						  ? "out"
						  : "in");
		},
		exec);

	addWriteOnlyAttr(
		chn, QStringLiteral("channel_enabled"), QString(), resource,
		[dig, pin](const QString &v) {
			const DIO_CHANNEL index = static_cast<DIO_CHANNEL>(pin);
			const bool enable = v.toInt() != 0;
			M2K_CALL(dig, "M2kDigital::enableChannel", dig->enableChannel(index, enable), M2K_ARG(index),
				 M2K_ARG(enable));
		},
		exec);
	return chn;
}

void addGroup(component::Device *dev, M2kDigital *dig, void *resource, int group, DioStatePtr state, ICmdExecutor *exec)
{
	const int base = dio::groupBase(group);

	auto *chn = new component::Channel(dev);
	chn->setId(QStringLiteral("group%1").arg(group));
	chn->setName(QStringLiteral("DIO %1 - %2").arg(base).arg(base + dio::PINS_PER_GROUP - 1));
	chn->setLabel(chn->name());
	chn->setIsOutput(false);

	auto *value = new component::Attribute(chn);
	value->setName(QStringLiteral("value"));
	value->setRange({0.0, 1.0, static_cast<double>(dio::GROUP_VALUE_MAX)});
	value->addReadCapability(new M2kAttributeReader(
		resource, [state, group]() { return QByteArray::number(dio::groupValue(state->gpo, group)); }, exec));
	value->addWriteCapability(new M2kAttributeWriter(
		resource,
		[dig, state, group, base](const QString &v) {
			state->gpo = dio::withGroupValue(state->gpo, group, static_cast<uint8_t>(v.toInt()));
			for(int i = 0; i < dio::PINS_PER_GROUP; ++i) {
				writePinValue(dig, *state, base + i);
			}
		},
		exec));

	auto *direction = new component::Attribute(chn);
	direction->setName(QStringLiteral("direction"));
	direction->setOptions({QLatin1String(dio::DIRECTION_OPTIONS[0]), QLatin1String(dio::DIRECTION_OPTIONS[1])});
	direction->addReadCapability(new M2kAttributeReader(
		resource,
		[state, group]() { return QByteArray(dio::groupIsOutput(state->direction, group) ? "out" : "in"); },
		exec));
	direction->addWriteCapability(new M2kAttributeWriter(
		resource,
		[dig, state, base](const QString &v) {
			const bool output = (v == QLatin1String("out"));
			for(int i = 0; i < dio::PINS_PER_GROUP; ++i) {
				state->direction = dio::withBit(state->direction, base + i, output);
				writePinDirection(dig, *state, base + i);
			}
		},
		exec));
}

void addDigitalDeviceAttrs(component::Device *dev, M2kDigital *dig, void *resource, DioStatePtr state,
			   ICmdExecutor *exec)
{
	auto *gpi = new component::Attribute(dev);
	gpi->setName(QStringLiteral("gpi"));
	gpi->addReadCapability(new M2kAttributeReader(
		resource,
		[dig]() {
			uint16_t word = 0;
			for(int i = 0; i < dio::PIN_COUNT; ++i) {
				const DIO_CHANNEL index = static_cast<DIO_CHANNEL>(i);
				const bool high = M2K_CALL(dig, "M2kDigital::getValueRaw", dig->getValueRaw(index),
							   M2K_ARG(index)) != DIO_LEVEL::LOW;
				word = dio::withBit(word, i, high);
			}
			return QByteArray::number(word);
		},
		exec));

	auto *outputEnabled = new component::Attribute(dev);
	outputEnabled->setName(QStringLiteral("output_enabled"));
	outputEnabled->setOptions({QStringLiteral("0"), QStringLiteral("1")});
	outputEnabled->addReadCapability(new M2kAttributeReader(
		resource, [state]() { return QByteArray::number(state->outputEnabled ? 1 : 0); }, exec));
	outputEnabled->addWriteCapability(new M2kAttributeWriter(
		resource,
		[dig, state](const QString &v) {
			const bool on = (v.toInt() != 0);
			if(state->outputEnabled == on) {
				return;
			}
			state->outputEnabled = on;
			for(int i = 0; i < dio::PIN_COUNT; ++i) {
				writePinDirection(dig, *state, i);
				writePinValue(dig, *state, i);
			}
		},
		exec));
}
void addDigitalBufferAttrs(component::Device *dev, M2kDigital *dig, void *resource, ICmdExecutor *exec)
{
	addReadWriteAttr(
		dev, QStringLiteral("sample_rate_in"), QStringLiteral("Hz"), resource,
		[dig]() {
			return QByteArray::number(M2K_CALL(dig, "M2kDigital::getSampleRateIn", dig->getSampleRateIn()),
						  'g', 12);
		},
		[dig](const QString &v) {
			const double samplerate = v.toDouble();
			M2K_CALL(dig, "M2kDigital::setSampleRateIn", dig->setSampleRateIn(samplerate),
				 M2K_ARG(samplerate));
		},
		exec);

	addReadWriteAttr(
		dev, QStringLiteral("sample_rate_out"), QStringLiteral("Hz"), resource,
		[dig]() {
			return QByteArray::number(
				M2K_CALL(dig, "M2kDigital::getSampleRateOut", dig->getSampleRateOut()), 'g', 12);
		},
		[dig](const QString &v) {
			const double samplerate = v.toDouble();
			M2K_CALL(dig, "M2kDigital::setSampleRateOut", dig->setSampleRateOut(samplerate),
				 M2K_ARG(samplerate));
		},
		exec);

	component::Attribute *cyclic = addReadWriteAttr(
		dev, QStringLiteral("cyclic"), QString(), resource,
		[dig]() {
			return QByteArray::number(M2K_CALL(dig, "M2kDigital::getCyclic", dig->getCyclic()) ? 1 : 0);
		},
		[dig](const QString &v) {
			const bool cyclic = v.toInt() != 0;
			M2K_CALL(dig, "M2kDigital::setCyclic", dig->setCyclic(cyclic), M2K_ARG(cyclic));
		},
		exec);
	cyclic->setOptions({QStringLiteral("0"), QStringLiteral("1")});

	addReadOnlyAttr(
		dev, QStringLiteral("nb_channels_in"), QString(), resource,
		[dig]() {
			return QByteArray::number(M2K_CALL(dig, "M2kDigital::getNbChannelsIn", dig->getNbChannelsIn()));
		},
		exec);

	addReadOnlyAttr(
		dev, QStringLiteral("nb_channels_out"), QString(), resource,
		[dig]() {
			return QByteArray::number(
				M2K_CALL(dig, "M2kDigital::getNbChannelsOut", dig->getNbChannelsOut()));
		},
		exec);

	addWriteOnlyAttr(
		dev, QStringLiteral("kernel_buffers_count_in"), QString(), resource,
		[dig](const QString &v) {
			const unsigned int count = static_cast<unsigned int>(v.toUInt());
			M2K_CALL(dig, "M2kDigital::setKernelBuffersCountIn", dig->setKernelBuffersCountIn(count),
				 M2K_ARG(count));
		},
		exec);
}

void buildDigital(component::Context *ctx, libm2k::context::M2k *m2k, ICmdExecutor *executor)
{
	component::Device *digitalDev = addDevice(ctx, QStringLiteral("digital"));

	M2kDigital *digital = nullptr;
	try {
		digital = m2k->getDigital();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "getDigital failed:" << e.what();
	}
	if(!digital) {
		qWarning(CAT_M2K_BUILDER) << "digital device left empty; the Digital I/O tool will be idle";
		return;
	}

	M2kHardwareTrigger *trig = triggerOf(digital);
	auto state = std::make_shared<dio::PinState>();
	QList<component::Channel *> pins;
	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		component::Channel *chn = addPin(digitalDev, digital, m2k, pin, state, executor);
		pins.append(chn);
		if(trig) {
			addDigitalTriggerPinAttr(chn, trig, m2k, pin, executor);
		}
	}
	for(int group = 0; group < dio::GROUP_COUNT; ++group) {
		addGroup(digitalDev, digital, m2k, group, state, executor);
	}
	addDigitalDeviceAttrs(digitalDev, digital, m2k, state, executor);
	addDigitalBufferAttrs(digitalDev, digital, m2k, executor);
	if(trig) {
		addDigitalTriggerDeviceAttrs(digitalDev, trig, probeTriggerCaps(trig), m2k, executor);
	}

	auto *din = new M2kDigitalInputStream(m2k, digital, executor, digitalDev);
	auto *dout = new M2kDigitalOutputStream(m2k, digital, executor, digitalDev);
	// The same pin channel is a member of both streams.
	for(component::Channel *chn : pins) {
		din->addChannel(chn);
		dout->addChannel(chn);
	}
}

void buildContextDevice(component::Context *ctx, libm2k::context::M2k *m2k, ICmdExecutor *executor)
{
	component::Device *contextDev = addDevice(ctx, QStringLiteral("context"));

	std::vector<std::string> names;
	try {
		names = m2k->getAvailableContextAttributes();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "getAvailableContextAttributes failed:" << e.what();
	}
	for(const std::string &name : names) {
		addReadOnlyAttr(
			contextDev, QString::fromStdString(name), QString(), m2k,
			[m2k, name]() {
				return QByteArray::fromStdString(M2K_CALL(m2k, "M2k::getContextAttributeValue",
									  m2k->getContextAttributeValue(name),
									  M2K_ARG(name)));
			},
			executor);
	}

	addWriteOnlyAttr(
		contextDev, QStringLiteral("timeout_ms"), QStringLiteral("ms"), m2k,
		[m2k](const QString &v) {
			const unsigned int timeout = v.toUInt();
			M2K_CALL(m2k, "M2k::setTimeout", m2k->setTimeout(timeout), M2K_ARG(timeout));
		},
		executor);
}

void buildDmm(component::Context *ctx, DMM *dmm, const QString &id, void *resource, ICmdExecutor *executor)
{
	component::Device *dmmDev = addDevice(ctx, id);

	std::vector<std::string> channels;
	try {
		channels = dmm->getAllChannels();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "getAllChannels failed on" << id << ":" << e.what();
	}
	for(const std::string &chnId : channels) {
		auto *chn = new component::Channel(dmmDev);
		chn->setId(QString::fromStdString(chnId));
		QString unit;
		try {
			const DMM_READING reading = dmm->readChannel(chnId);
			chn->setName(QString::fromStdString(reading.name.empty() ? chnId : reading.name));
			unit = QString::fromStdString(reading.unit_symbol);
		} catch(const std::exception &e) {
			qWarning(CAT_M2K_BUILDER) << "readChannel failed on" << id << chnId.c_str() << ":" << e.what();
			chn->setName(chn->id());
		}
		chn->setLabel(chn->name());
		chn->setIsOutput(false);

		addReadOnlyAttr(
			chn, QStringLiteral("value"), unit, resource,
			[dmm, chnId]() { return QByteArray::number(dmm->readChannel(chnId).value, 'g', 12); },
			executor);
	}
}

void buildDmms(component::Context *ctx, libm2k::context::M2k *m2k, ICmdExecutor *executor)
{
	std::vector<DMM *> dmms;
	try {
		dmms = m2k->getAllDmm();
	} catch(const std::exception &e) {
		qWarning(CAT_M2K_BUILDER) << "getAllDmm failed:" << e.what();
		return;
	}

	QSet<QString> used;
	for(DMM *dmm : dmms) {
		if(!dmm) {
			continue;
		}
		QString name;
		try {
			name = QString::fromStdString(dmm->getName());
		} catch(const std::exception &e) {
			qWarning(CAT_M2K_BUILDER) << "DMM getName failed:" << e.what();
		}
		QString id = QStringLiteral("dmm-") + (name.isEmpty() ? QStringLiteral("unnamed") : name);
		for(int n = 1; used.contains(id); ++n) {
			id = QStringLiteral("dmm-%1-%2").arg(name).arg(n);
		}
		used.insert(id);
		buildDmm(ctx, dmm, id, m2k, executor);
	}
}

void invalidateInputsOnReconfigure(component::Context *ctx)
{
	const auto device = [ctx](const QString &id) {
		return ctx->findChild<component::Device *>(id, Qt::FindDirectChildrenOnly);
	};
	component::Device *analogIn = device(QStringLiteral("analog-in"));
	component::Device *analogOut = device(QStringLiteral("analog-out"));
	component::Device *powerSupply = device(QStringLiteral("power-supply"));
	component::Device *digital = device(QStringLiteral("digital"));

	QPointer<M2kInputStream> ain = component::streamAt<M2kInputStream>(analogIn);
	QPointer<M2kDigitalInputStream> din = component::streamAt<M2kDigitalInputStream>(digital);
	if(!ain && !din) {
		return;
	}
	const auto invalidateInputs = [ain, din]() {
		if(ain) {
			ain->invalidate();
		}
		if(din) {
			din->invalidate();
		}
	};

	for(component::Device *dev : {analogIn, analogOut, powerSupply, digital}) {
		if(!dev) {
			continue;
		}
		const QList<component::Attribute *> attrs = dev->findChildren<component::Attribute *>();
		for(component::Attribute *attr : attrs) {
			component::AttributeWriter *writer = attr->writeCapability();
			if(!writer) {
				continue;
			}
			QObject::connect(writer, &component::AttributeWriter::writeSucceeded, ctx, invalidateInputs);
		}
	}

	if(auto *aout = component::streamAt<M2kOutputStream>(analogOut)) {
		QObject::connect(aout, &component::OutputStream::pushSucceeded, ctx, invalidateInputs);
		QObject::connect(aout, &component::Stream::openedChanged, ctx, invalidateInputs);
	}
	if(auto *dout = component::streamAt<M2kDigitalOutputStream>(digital)) {
		QObject::connect(dout, &component::OutputStream::pushSucceeded, ctx, invalidateInputs);
		QObject::connect(dout, &component::Stream::openedChanged, ctx, invalidateInputs);
	}
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

	if(!buildAnalogIn(m2kCtx, m2k, executor)) {
		return false;
	}
	buildAnalogOut(m2kCtx, m2k, executor);
	buildPowerSupply(m2kCtx, m2k, executor);
	buildDigital(m2kCtx, m2k, executor);
	buildContextDevice(m2kCtx, m2k, executor);
	buildDmms(m2kCtx, m2k, executor);
	invalidateInputsOnReconfigure(m2kCtx);

	return true;
}
