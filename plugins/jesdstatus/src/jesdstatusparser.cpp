/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "jesdstatusparser.h"
#include <QString>
#include <QDebug>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QLoggingCategory>

using namespace scopy::jesdstatus;

Q_LOGGING_CATEGORY(CAT_JESDPARSER, "JesdParser");
#define MAX_JESD_ATTR_SIZE 8192

JesdStatusParser::JesdStatusParser(iio_device *dev, QObject *parent)
	: QObject(parent)
	, m_dev(dev)
	, m_laneCount(0)
	, m_encoder(JESD204_UNKNOWN)
{
	readEncoder();
	unsigned int attrCount = iio_device_get_attrs_count(m_dev);
	for(unsigned i = 0; i < attrCount; i++) {
		QString attr = iio_device_get_attr(m_dev, i);
		if(!attr.isEmpty() && attr.contains("lane")) {
			m_laneCount++;
		}
	}
}

JesdStatusParser::~JesdStatusParser() {}

void JesdStatusParser::update()
{
	readAllLaneStatus();
	readStatus();
	Q_EMIT finished();
}

ENCODER_TYPE JesdStatusParser::getEncoder() const { return m_encoder; }

unsigned int JesdStatusParser::getLaneCount() { return m_laneCount; }

QPair<QString, VISUAL_STATUS> JesdStatusParser::getErrors(unsigned int lane)
{
	QPair<QString, VISUAL_STATUS> ret;
	if(lane >= m_allLaneStatus.size()) {
		return ret;
	}
	int laneErr = m_allLaneStatus.value(lane).lane_errors;
	ret.first = (laneErr < 0) ? "" : QString::number(laneErr);
	ret.second = (laneErr) ? C_ERR : C_GOOD;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getExtMultiBlockAlignment(unsigned int lane)
{
	QPair<QString, VISUAL_STATUS> ret;
	if(lane >= m_allLaneStatus.size()) {
		return ret;
	}
	if(m_encoder != JESD204_64B66B) {
		return ret;
	}

	QString mbAlign = m_allLaneStatus.value(lane).ext_multiblock_align_state;
	ret.first = QString(mbAlign);
	ret.second = (mbAlign == "EMB_LOCK") ? C_GOOD : C_ERR;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getLaneLatency(unsigned int lane)
{
	QPair<QString, VISUAL_STATUS> ret;
	if(lane >= m_allLaneStatus.size()) {
		return ret;
	}

	auto currentLane = m_allLaneStatus.value(lane);
	if(currentLane.lane_latency_multiframes < 0) {
		ret.first = "";
		ret.second = C_ERR;
		return ret;
	}
	unsigned int octetsPerMultiframe = currentLane.k * currentLane.f;
	unsigned int latency =
		octetsPerMultiframe * currentLane.lane_latency_multiframes + currentLane.lane_latency_octets;

	if((latency - m_minLatency) >= octetsPerMultiframe) {
		ret.second = C_ERR;
	} else if((latency - m_minLatency) > (octetsPerMultiframe / 2)) {
		ret.second = C_CRIT;
	} else {
		ret.second = C_GOOD;
	}

	ret.first = QString("%1/%2").arg(currentLane.lane_latency_multiframes).arg(currentLane.lane_latency_octets);
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getCgsState(unsigned int lane)
{
	QPair<QString, VISUAL_STATUS> ret;
	if(lane >= m_allLaneStatus.size()) {
		return ret;
	}
	QString cgsState = m_allLaneStatus.value(lane).cgs_state;
	ret.first = QString(cgsState);
	ret.second = (cgsState == "DATA") ? C_GOOD : C_ERR;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getInitFrameSync(unsigned int lane)
{
	QPair<QString, VISUAL_STATUS> ret;
	if(lane >= m_allLaneStatus.size()) {
		return ret;
	}
	QString initFS = m_allLaneStatus.value(lane).init_frame_sync;
	ret.first = QString(initFS);
	ret.second = (initFS == "YES") ? C_GOOD : C_ERR;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getInitLaneAlignSeq(unsigned int lane)
{
	QPair<QString, VISUAL_STATUS> ret;
	if(lane >= m_allLaneStatus.size()) {
		return ret;
	}
	QString initLaneAlign = m_allLaneStatus.value(lane).init_lane_align_seq;
	ret.first = QString(initLaneAlign);
	ret.second = (initLaneAlign == "YES") ? C_GOOD : C_ERR;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getLinkState()
{
	QPair<QString, VISUAL_STATUS> ret;
	QString linkState = m_jesd204_status.link_state;
	ret.first = QString(linkState);
	ret.second = (linkState == "enabled") ? C_GOOD : C_ERR;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getLinkStatus()
{
	QPair<QString, VISUAL_STATUS> ret;
	QString linkStatus = m_jesd204_status.link_status;
	ret.first = QString(linkStatus);
	ret.second = (linkStatus == "DATA") ? C_GOOD : C_ERR;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getMeasuredLinkClock()
{
	QPair<QString, VISUAL_STATUS> ret;
	bool ok;
	float measured = 0.0;
	float reported = 0.0;
	QString measuredLinkClock = m_jesd204_status.measured_link_clock;
	QString reportedLinkClock = m_jesd204_status.reported_link_clock;

	measured = measuredLinkClock.toFloat(&ok);
	if(!ok) {
		return ret;
	}
	reported = reportedLinkClock.toFloat(&ok);
	if(!ok) {
		return ret;
	}

	if((measured > (reported * (1 + (m_clockAccuracy / 1000000.0)))) ||
	   (measured < (reported * (1 - (m_clockAccuracy / 1000000.0))))) {
		ret.second = C_ERR;
	} else {
		ret.second = C_GOOD;
	}
	ret.first = measuredLinkClock;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getReportedLinkClock()
{
	QPair<QString, VISUAL_STATUS> ret;
	QString reportedLinkClock = m_jesd204_status.reported_link_clock;
	ret.first = reportedLinkClock;
	ret.second = C_NORM;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getMeasuredDeviceClock()
{
	QPair<QString, VISUAL_STATUS> ret;
	bool ok;
	float measured = 0.0;
	float reported = 0.0;
	//	float div40 = 0.0;
	QString measuredDeviceClock = m_jesd204_status.measured_device_clock;
	QString reportedDeviceClock = m_jesd204_status.reported_device_clock;

	ret.first = measuredDeviceClock;
	measured = measuredDeviceClock.toFloat(&ok);
	if(!ok) {
		ret.second = C_NORM;
		return ret;
	}
	reported = reportedDeviceClock.toFloat(&ok);
	if(!ok) {
		ret.second = C_NORM;
		return ret;
	}

	if((measured > (reported * (1 + (m_clockAccuracy / 1000000.0)))) ||
	   (measured < (reported * (1 - (m_clockAccuracy / 1000000.0))))) {
		ret.second = C_ERR;
	} else {
		ret.second = C_GOOD;
	}
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getReportedDeviceClock()
{
	QPair<QString, VISUAL_STATUS> ret;
	bool ok;
	float reported = 0.0;
	float div40 = 0.0;
	QString reportedDeviceClock = m_jesd204_status.reported_device_clock;
	QString desiredDeviceClock = m_jesd204_status.desired_device_clock;

	ret.first = reportedDeviceClock;
	reported = reportedDeviceClock.toFloat(&ok);
	if(!ok) {
		ret.second = C_NORM;
		return ret;
	}
	div40 = desiredDeviceClock.toFloat(&ok);
	if(!ok) {
		ret.second = C_NORM;
		return ret;
	}

	if((reported > (div40 * (1 + (m_clockAccuracy / 1000000.0)))) ||
	   (reported < (div40 * (1 - (m_clockAccuracy / 1000000.0))))) {
		ret.second = C_ERR;
	} else {
		ret.second = C_GOOD;
	}
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getDesiredDeviceClock()
{
	QPair<QString, VISUAL_STATUS> ret;
	QString desiredDeviceClock = m_jesd204_status.desired_device_clock;
	ret.first = desiredDeviceClock;
	ret.second = C_NORM;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getLaneRate()
{
	QPair<QString, VISUAL_STATUS> ret;
	QString laneRate = m_jesd204_status.lane_rate;
	ret.first = laneRate;
	ret.second = C_NORM;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getLaneRateDiv()
{
	QPair<QString, VISUAL_STATUS> ret;
	bool ok;
	float reported = 0.0;
	float div40 = 0.0;
	QString reportedLinkClock = m_jesd204_status.reported_link_clock;
	QString laneRateDiv = m_jesd204_status.lane_rate_div;

	reported = reportedLinkClock.toFloat(&ok);
	if(!ok) {
		return ret;
	}
	div40 = laneRateDiv.toFloat(&ok);
	if(!ok) {
		return ret;
	}

	if((reported > (div40 * (1 + (m_clockAccuracy / 1000000.0)))) ||
	   (reported < (div40 * (1 - (m_clockAccuracy / 1000000.0))))) {
		ret.second = C_ERR;
	} else {
		ret.second = C_GOOD;
	}
	ret.first = laneRateDiv;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getLmfcRate()
{
	QPair<QString, VISUAL_STATUS> ret;
	QString lmfcRate = m_jesd204_status.lmfc_rate;
	ret.first = lmfcRate;
	ret.second = C_NORM;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getSysrefCaptured()
{
	QPair<QString, VISUAL_STATUS> ret;
	QString sysrefCaptured = m_jesd204_status.sysref_captured;
	ret.first = sysrefCaptured;
	ret.second = (sysrefCaptured == "No") ? C_ERR : C_GOOD;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getSysrefAlignmentError()
{
	QPair<QString, VISUAL_STATUS> ret;
	QString sysrefAlignment = m_jesd204_status.sysref_alignment_error;
	ret.first = sysrefAlignment;
	ret.second = (sysrefAlignment == "Yes") ? C_ERR : C_GOOD;
	return ret;
}

QPair<QString, VISUAL_STATUS> JesdStatusParser::getSyncState()
{
	QPair<QString, VISUAL_STATUS> ret;
	if(m_encoder != JESD204_8B10B) {
		return ret;
	}

	QString syncState = m_jesd204_status.sync_state;
	ret.first = syncState;
	ret.second = (syncState == "deasserted") ? C_GOOD : C_ERR;
	return ret;
}

int JesdStatusParser::extractLaneNumber(const QString &text)
{
	int laneNb = -1;
	QRegularExpression regex("lane(\\d+)_info");
	QRegularExpressionMatch match = regex.match(text);

	if(match.hasMatch()) {
		bool ok = true;
		int value = match.captured(1).toInt(&ok);
		if(ok) {
			laneNb = value;
		}
	}
	return laneNb;
}

void JesdStatusParser::readEncoder()
{
	char encoder[MAX_JESD_ATTR_SIZE];

	// If the encoder is not found, default to 8b10b. It might be an
	// older kernel that only supports jesd204b
	const char *encoderAttr = iio_device_find_attr(m_dev, "encoder");
	if(!encoderAttr) {
		m_encoder = JESD204_8B10B;
	}

	int ret = iio_device_attr_read(m_dev, "encoder", encoder, MAX_JESD_ATTR_SIZE);
	if(ret < 0) {
		qDebug(CAT_JESDPARSER) << "There is an issue reading the JESD204 status encoder. Aborting...";
		m_encoder = JESD204_UNKNOWN;
	} else if(!QString(encoder).compare(m_encoderTypes.value(JESD204_8B10B), Qt::CaseInsensitive)) {
		m_encoder = JESD204_8B10B;
	} else {
		m_encoder = JESD204_64B66B;
	}
}

void JesdStatusParser::readLaneStatus(QString laneAttr)
{
	JESD204B_LANEINFO m_jesd204_lanestatus;
	QString laneStatus = "";
	int laneId = regexMatchUInt(laneAttr, QRegularExpression("lane(\\d+)_info"));

	if(m_encoder == JESD204_UNKNOWN) {
		return;
	}

	char buf[MAX_JESD_ATTR_SIZE];
	int ret = iio_device_attr_read(m_dev, laneAttr.toUtf8(), buf, MAX_JESD_ATTR_SIZE);
	if(ret < 0) {
		qDebug(CAT_JESDPARSER) << "There is an issue reading the JESD204 lane status. Aborting...";
		return;
	}
	laneStatus = QString(buf);

	m_jesd204_lanestatus.lane_errors = regexMatchUInt(laneStatus, QRegularExpression("Errors: (\\S+)[ \\n]?"));
	if(m_encoder == JESD204_64B66B) {
		m_jesd204_lanestatus.ext_multiblock_align_state = regexMatch(
			laneStatus, QRegularExpression("State of Extended multiblock alignment:(\\S+)[ \\n]?"));
	}
	m_jesd204_lanestatus.cgs_state = regexMatch(laneStatus, QRegularExpression("CGS state: (\\S+)[ \\n]?"));
	m_jesd204_lanestatus.init_frame_sync =
		regexMatch(laneStatus, QRegularExpression("Initial Frame Synchronization: (\\S+)[ \\n]?"));

	QList<QString> multi_frames_and_octets = regexMatchMultiple(
		laneStatus, QRegularExpression("Lane Latency: (\\S+) Multi-frames and  (\\S+) Octets[ \\n]?"), 2);
	if(multi_frames_and_octets.size() == 2) {
		bool ok = false;
		unsigned int multiframes = multi_frames_and_octets.at(0).toUInt(&ok);
		if(ok) {
			m_jesd204_lanestatus.lane_latency_multiframes = multiframes;
		}
		unsigned int octets = multi_frames_and_octets.at(1).toUInt(&ok);
		if(ok) {
			m_jesd204_lanestatus.lane_latency_octets = octets;
		}
	} else {
		qDebug(CAT_JESDPARSER) << "There is an issue reading the JESD204 lane status latency.";
	}

	m_jesd204_lanestatus.init_lane_align_seq =
		regexMatch(laneStatus, QRegularExpression("Initial Lane Alignment Sequence: (\\S+)[ \\n]?"));

	QList<unsigned int> id_list = regexMatchMultipleUInt(
		laneStatus,
		QRegularExpression("DID: (\\S+), BID: (\\S+), LID: (\\S+), L: (\\S+), SCR: (\\S+), F: (\\S+)[ \\n]?"),
		6);
	if(id_list.size() == 6) {
		m_jesd204_lanestatus.did = id_list.at(0);
		m_jesd204_lanestatus.bid = id_list.at(1);
		m_jesd204_lanestatus.lid = id_list.at(2);
		m_jesd204_lanestatus.l = id_list.at(3);
		m_jesd204_lanestatus.scr = id_list.at(4);
		m_jesd204_lanestatus.f = id_list.at(5);
	} else {
		qDebug(CAT_JESDPARSER) << "There is an issue reading the JESD204 IDs.";
	}

	QList<unsigned int> frame_control_list = regexMatchMultipleUInt(
		laneStatus,
		QRegularExpression(
			"K: (\\S+), M: (\\S+), N: (\\S+), CS: (\\S+), N': (\\S+), S: (\\S+), HD: (\\S+)[ \\n]?"),
		7);
	if(frame_control_list.size() == 7) {
		m_jesd204_lanestatus.k = frame_control_list.at(0);
		m_jesd204_lanestatus.m = frame_control_list.at(1);
		m_jesd204_lanestatus.n = frame_control_list.at(2);
		m_jesd204_lanestatus.cs = frame_control_list.at(3);
		m_jesd204_lanestatus.nd = frame_control_list.at(4);
		m_jesd204_lanestatus.s = frame_control_list.at(5);
		m_jesd204_lanestatus.hd = frame_control_list.at(6);
	} else {
		qDebug(CAT_JESDPARSER) << "There is an issue reading the JESD204 frame info.";
	}

	QList<unsigned int> checksum_list =
		regexMatchMultipleUInt(laneStatus, QRegularExpression("FCHK: (\\S+), CF: (\\S+)[ \\n]?"), 2);
	if(checksum_list.size() == 2) {
		m_jesd204_lanestatus.fchk = checksum_list.at(0);
		m_jesd204_lanestatus.cf = checksum_list.at(1);
	} else {
		qDebug(CAT_JESDPARSER) << "There is an issue reading the JESD204 checksum info.";
	}

	QList<unsigned int> adjustment_list = regexMatchMultipleUInt(
		laneStatus,
		QRegularExpression(
			"ADJCNT: (\\S+), PHADJ: (\\S+), ADJDIR: (\\S+), JESDV: (\\S+), SUBCLASS: (\\S+)[ \\n]?"),
		5);
	if(adjustment_list.size() == 5) {
		m_jesd204_lanestatus.adjcnt = adjustment_list.at(0);
		m_jesd204_lanestatus.phyadj = adjustment_list.at(1);
		m_jesd204_lanestatus.adjdir = adjustment_list.at(2);
		m_jesd204_lanestatus.jesdv = adjustment_list.at(3);
		m_jesd204_lanestatus.subclassv = adjustment_list.at(4);
	} else {
		qDebug(CAT_JESDPARSER) << "There is an issue reading the JESD204 adjustment info.";
	}

	m_allLaneStatus.insert(laneId, m_jesd204_lanestatus);
}

void JesdStatusParser::readAllLaneStatus()
{
	if(m_encoder == JESD204_UNKNOWN) {
		return;
	}

	unsigned int attrCount = iio_device_get_attrs_count(m_dev);
	for(unsigned i = 0; i < attrCount; i++) {
		QString attr = iio_device_get_attr(m_dev, i);
		if(!attr.isEmpty() && attr.contains("lane")) {
			readLaneStatus(attr);
		}
	}

	int minLatency = 0;
	int octetsPerMultiframe = 0;
	unsigned int laneCount = m_allLaneStatus.size();
	auto lanes = m_allLaneStatus.keys();
	for(unsigned int i = 0, minLatency = INT_MAX; i < laneCount; i++) {
		auto lane = m_allLaneStatus.value(lanes.at(i));
		octetsPerMultiframe = lane.k * lane.f;

		if(lane.lane_latency_octets < 0 || lane.lane_latency_multiframes < 0) {
			continue;
		}

		minLatency = std::min(minLatency,
				      octetsPerMultiframe * (unsigned int)lane.lane_latency_multiframes +
					      lane.lane_latency_octets);
	}
	m_minLatency = minLatency;
}

void JesdStatusParser::readStatus()
{
	QString status = "";
	const QString notAvailable = "N/A";
	if(m_encoder == JESD204_UNKNOWN) {
		return;
	}

	char buf[MAX_JESD_ATTR_SIZE];
	int ret = iio_device_attr_read(m_dev, "status", buf, MAX_JESD_ATTR_SIZE);
	if(ret < 0) {
		qDebug(CAT_JESDPARSER) << "There is an issue reading the JESD204 status. Aborting...";
		return;
	}
	status = QString(buf);

	m_jesd204_status.link_state = regexMatch(status, QRegularExpression("Link is (\\S+)"));
	m_jesd204_status.measured_link_clock =
		regexMatch(status, QRegularExpression("Measured Link Clock: (\\S+) MHz"));
	m_jesd204_status.reported_link_clock =
		regexMatch(status, QRegularExpression("Reported Link Clock: (\\S+) MHz"));
	m_jesd204_status.measured_device_clock =
		regexMatch(status, QRegularExpression("Measured Device Clock: (\\S+) MHz"), notAvailable);
	if(m_jesd204_status.measured_device_clock == notAvailable) {
		m_jesd204_status.reported_device_clock = notAvailable;
		m_jesd204_status.desired_device_clock = notAvailable;
	} else {
		m_jesd204_status.reported_device_clock =
			regexMatch(status, QRegularExpression("Reported Device Clock: (\\S+) MHz"), notAvailable);
		m_jesd204_status.desired_device_clock =
			regexMatch(status, QRegularExpression("Desired Device Clock: (\\S+) MHz"), notAvailable);
	}
	m_jesd204_status.lane_rate = regexMatch(status, QRegularExpression("Lane Rate: (\\S+) MHz"));
	if(m_jesd204_status.lane_rate == "") {
		m_jesd204_status.external_reset =
			regexMatch(status, QRegularExpression("External reset is (\\S+)[ \\n]?"));
	}

	if(m_encoder == JESD204_8B10B) {
		m_jesd204_status.lane_rate_div =
			regexMatch(status, QRegularExpression("Lane rate / 40: (\\S+) MHz[ \\n]?"));
		m_jesd204_status.lmfc_rate = regexMatch(status, QRegularExpression("LMFC rate: (\\S+) MHz[ \\n]?"));
		m_jesd204_status.sync_state = regexMatch(status, QRegularExpression("SYNC~: (\\S+)[ \\n]?"));
	} else {
		m_jesd204_status.lane_rate_div =
			regexMatch(status, QRegularExpression("Lane rate / 66: (\\S+) MHz[ \\n]?"));
		m_jesd204_status.lmfc_rate = regexMatch(status, QRegularExpression("LEMC rate: (\\S+) MHz[ \\n]?"));
	}

	m_jesd204_status.link_status = regexMatch(status, QRegularExpression("Link status: (\\S+)[ \\n]?"));
	m_jesd204_status.sysref_captured = regexMatch(status, QRegularExpression("SYSREF captured: (\\S+)[ \\n]?"));
	m_jesd204_status.sysref_alignment_error =
		regexMatch(status, QRegularExpression("SYSREF alignment error: (\\S+)[ \\n]?"));
}

long JesdStatusParser::regexMatchUInt(QString container, QRegularExpression regex)
{
	bool ok = false;
	QString match = regexMatch(container, regex);
	unsigned int value = match.toUInt(&ok);
	if(!ok) {
		return -1;
	}
	return value;
}

QList<unsigned int> JesdStatusParser::regexMatchMultipleUInt(QString container, QRegularExpression regex, int count)
{
	bool ok = false;
	QList<unsigned int> capture = {};
	QList<QString> match = regexMatchMultiple(container, regex, count);
	for(QString &m : match) {
		unsigned int value = m.toUInt(&ok);
		if(ok) {
			capture.push_back(value);
		}
		ok = false;
	}
	return capture;
}

QList<QString> JesdStatusParser::regexMatchMultiple(QString container, QRegularExpression regex, int count)
{
	QList<QString> capture = {};
	QRegularExpressionMatch match = regex.match(container);
	if(match.hasMatch()) {
		for(int i = 1; i < count; i++) {
			if(i <= match.capturedLength()) {
				capture.push_back(match.captured(i));
			}
		}
	}
	return capture;
}

QString JesdStatusParser::regexMatch(QString container, QRegularExpression regex, QString init)
{
	QString capture = init;
	QRegularExpressionMatch match = regex.match(container);
	if(match.hasMatch()) {
		capture = match.captured(1);
	}
	return capture;
}
