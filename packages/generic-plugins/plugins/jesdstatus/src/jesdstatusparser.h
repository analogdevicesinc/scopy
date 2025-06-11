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

#ifndef JESDSTATUSPARSER_H
#define JESDSTATUSPARSER_H

#include "scopy-jesdstatus_export.h"
#include <QString>
#include <QMap>
#include <QObject>
#include <iio.h>

namespace scopy::jesdstatus {

enum ENCODER_TYPE : int
{
	JESD204_8B10B = 0,
	JESD204_64B66B = 1,
	JESD204_UNKNOWN = -1
};

enum VISUAL_STATUS : int
{
	C_NORM = 1,
	C_GOOD = 2,
	C_ERR = 3,
	C_CRIT = 4,
	C_OPT = 5,
};

class SCOPY_JESDSTATUS_EXPORT JesdStatusParser : public QObject
{
	Q_OBJECT
public:
	JesdStatusParser(struct iio_device *dev, QObject *parent);
	virtual ~JesdStatusParser();

	ENCODER_TYPE getEncoder() const;

	/** Lane status section **/
	unsigned int getLaneCount();
	QPair<QString, VISUAL_STATUS> getErrors(unsigned int lane);
	QPair<QString, VISUAL_STATUS> getExtMultiBlockAlignment(unsigned int lane);
	QPair<QString, VISUAL_STATUS> getLaneLatency(unsigned int lane);
	QPair<QString, VISUAL_STATUS> getCgsState(unsigned int lane);
	QPair<QString, VISUAL_STATUS> getInitFrameSync(unsigned int lane);
	QPair<QString, VISUAL_STATUS> getInitLaneAlignSeq(unsigned int lane);

	/** Status section**/
	QPair<QString, VISUAL_STATUS> getLinkState();
	QPair<QString, VISUAL_STATUS> getLinkStatus();
	QPair<QString, VISUAL_STATUS> getMeasuredLinkClock();
	QPair<QString, VISUAL_STATUS> getReportedLinkClock();
	QPair<QString, VISUAL_STATUS> getMeasuredDeviceClock();
	QPair<QString, VISUAL_STATUS> getReportedDeviceClock();
	QPair<QString, VISUAL_STATUS> getDesiredDeviceClock();
	QPair<QString, VISUAL_STATUS> getLaneRate();
	QPair<QString, VISUAL_STATUS> getLaneRateDiv();
	QPair<QString, VISUAL_STATUS> getLmfcRate();
	QPair<QString, VISUAL_STATUS> getSysrefCaptured();
	QPair<QString, VISUAL_STATUS> getSysrefAlignmentError();
	QPair<QString, VISUAL_STATUS> getSyncState();

public Q_SLOTS:
	void update();

Q_SIGNALS:
	void finished();

private:
	struct iio_device *m_dev;
	unsigned int m_laneCount;
	ENCODER_TYPE m_encoder;
	int m_minLatency;
	const int m_clockAccuracy = 200;
	const QMap<ENCODER_TYPE, QString> m_encoderTypes = {{JESD204_8B10B, "8B10B"}, {JESD204_64B66B, "64B66B"}};

	struct JESD204B_JESD204_STATUS
	{

		QString link_state;
		QString measured_link_clock;
		QString reported_link_clock;
		QString measured_device_clock;
		QString reported_device_clock;
		QString desired_device_clock;
		QString lane_rate;
		QString lane_rate_div;
		QString lmfc_rate;
		QString sync_state;
		QString link_status;
		QString sysref_captured;
		QString sysref_alignment_error;
		QString external_reset;
	} m_jesd204_status;

	struct JESD204B_LANEINFO
	{
		unsigned int did;	/* DID Device ID */
		unsigned int bid;	/* BID Bank ID */
		unsigned int lid;	/* LID Lane ID */
		unsigned int l;		/* Number of Lanes per Device */
		unsigned int scr;	/* SCR Scrambling Enabled */
		unsigned int f;		/* Octets per Frame */
		unsigned int k;		/* Frames per Multiframe */
		unsigned int m;		/* Converters per Device */
		unsigned int n;		/* Converter Resolution */
		unsigned int cs;	/* Control Bits per Sample */
		unsigned int s;		/* Samples per Converter per Frame Cycle */
		unsigned int nd;	/* Total Bits per Sample */
		unsigned int hd;	/* High Density Format */
		unsigned int fchk;	/* Checksum */
		unsigned int cf;	/* Control Words per Frame Cycle per Link */
		unsigned int adjcnt;	/* ADJCNT Adjustment step count */
		unsigned int phyadj;	/* PHYADJ Adjustment request */
		unsigned int adjdir;	/* ADJDIR Adjustment direction */
		unsigned int jesdv;	/* JESD204 Version */
		unsigned int subclassv; /* JESD204 subclass version */

		unsigned long fc;

		int lane_errors = -1;
		int lane_latency_multiframes = -1;
		int lane_latency_octets = -1;

		QString cgs_state;
		QString init_frame_sync;
		QString init_lane_align_seq;
		QString ext_multiblock_align_state;
	};

	QMap<unsigned int, JESD204B_LANEINFO> m_allLaneStatus;

private:
	int extractLaneNumber(const QString &text);
	void readEncoder();
	void readLaneStatus(QString laneAttr);
	void readAllLaneStatus();
	void readStatus();
	QString regexMatch(QString container, QRegularExpression regex, QString init = "");
	long regexMatchUInt(QString container, QRegularExpression regex);
	QList<QString> regexMatchMultiple(QString container, QRegularExpression regex, int count);
	QList<unsigned int> regexMatchMultipleUInt(QString container, QRegularExpression regex, int count);
};

} // namespace scopy::jesdstatus
#endif // JESDSTATUSPARSER_H
