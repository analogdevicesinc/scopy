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

#ifndef ADCINSTRUMENTCONTROLLER_H
#define ADCINSTRUMENTCONTROLLER_H

#include "scopy-adc_export.h"
#include "timeplotcomponent.h"
#include "measurecomponent.h"
#include <QMap>
#include <pluginbase/resourcemanager.h>
#include <pluginbase/toolmenuentry.h>
#include <gui/plotmanager.h>
#include <gui/toolcomponent.h>
#include "adcacquisitionmanager.h"

namespace scopy {

class ChannelIdProvider;

namespace adc {

class ADCInstrument;

class SCOPY_ADC_EXPORT ADCInstrumentController : public QObject,
						 public AcqNodeChannelAware,
						 public MetaComponent,
						 public ResourceUser
{
	Q_OBJECT
public:
	ADCInstrumentController(ToolMenuEntry *tme, QString uri, QString name, AcqTreeNode *tree,
				QObject *parent = nullptr);
	virtual ~ADCInstrumentController();

	ChannelIdProvider *getChannelIdProvider();

	virtual void setEnableAddRemovePlot(bool);
	virtual void setEnableAddRemoveInstrument(bool);

public:
	ADCInstrument *ui() const;

	bool isMainInstrument() const;
	void setIsMainInstrument(bool newIsMainInstrument);

public Q_SLOTS:
	virtual void init();
	virtual void deinit();
	virtual void onStart() override;
	virtual void onStop() override;

	virtual void start();
	virtual void stop() override;

protected Q_SLOTS:
	virtual void stopUpdates();
	virtual void startUpdates();

	virtual void setSingleShot(bool b);
	virtual void setFrameRate(double val);
	virtual void updateFrameRate();
	virtual void handlePreferences(QString key, QVariant v);

	virtual void updateData();
	virtual void update();

Q_SIGNALS:
	void requestStart();
	void requestStop();
	void requestStartLater();
	void requestStopLater();
	void requestDisconnect();

protected:
	void setupChannelMeasurement(PlotManager *c, ChannelComponent *ch);

	ADCInstrument *m_ui;
	PlotManager *m_plotComponentManager;
	MapStackedWidget *plotStack;

	MeasureComponent *m_measureComponent;

	ChannelIdProvider *chIdP;
	CollapsableMenuControlButton *m_otherCMCB;

	QFuture<void> m_refillFuture;
	QFutureWatcher<void> *m_fw;
	QTimer *m_plotTimer;

	bool m_started;

	AcqTreeNode *m_tree;
	DataProvider *m_dataProvider;
	QMap<AcqTreeNode *, ToolComponent *> m_acqNodeComponentMap;

	bool m_refreshTimerRunning;
	bool m_isMainInstrument = false;

	ToolMenuEntry *m_tme;
	QString m_uri;
};

} // namespace adc
} // namespace scopy
#endif // ADCINSTRUMENTCONTROLLER_H
