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

#ifndef GRTIMESINKCOMPONENT_H
#define GRTIMESINKCOMPONENT_H

#include <QObject>
#include "adcinterfaces.h"
#include <gui/toolcomponent.h>
#include <gui/channelcomponent.h>
#include <gr-util/time_sink_f.h>
#include <synccontroller.h>
#include "adcacquisitionmanager.h"

namespace scopy {
namespace adc {

class GRTimeSinkComponent : public QObject, public DataProvider, public SyncInstrument
{
	Q_OBJECT
public:
	GRTimeSinkComponent(QString name, GRTopBlockNode *t, QObject *parent = nullptr);
	~GRTimeSinkComponent();

	bool finished() override;
	const QString &name() const;

public Q_SLOTS:
	void connectSignalPaths();
	void tearDownSignalPaths();

	virtual void onArm() override;
	virtual void onDisarm() override;
	virtual void setSyncMode(bool b) override;
	virtual void setSyncController(SyncController *s) override;
	virtual bool syncMode() override;

	void init();
	void deinit();

	virtual bool start() override;
	virtual void stop() override;

	virtual size_t updateData() override;
	virtual void setSingleShot(bool) override;
	virtual void setData(bool copy = false) override;

	SamplingInfo samplingInfo();
	void setSamplingInfo(SamplingInfo p);

	void addChannel(GRChannel *ch);
	void removeChannel(GRChannel *c);

	void setSyncSingleShot(bool) override;
	void setSyncBufferSize(uint32_t) override;

Q_SIGNALS:
	void arm();
	void disarm();

	void ready();
	void finish();

	void requestRebuild();
	void requestSingleShot(bool);
	void requestBufferSize(uint32_t);

private:
	std::mutex refillMutex;
	time_sink_f::sptr time_sink;
	QMap<QString, int> time_channel_map;
	SamplingInfo m_samplingInfo;

	GRTopBlockNode *m_node;
	GRTopBlock *m_top;

	bool m_singleShot;
	bool m_syncMode;
	bool m_armed;

	SyncController *m_sync;

	QList<GRChannel *> m_channels;
	QString m_name;

	// SampleRateProvider interface
};
} // namespace adc
} // namespace scopy

#endif // GRTIMESINKCOMPONENT_H
