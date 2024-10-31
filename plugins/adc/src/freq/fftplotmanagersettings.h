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

#ifndef FFTPLOTMANAGERSETTINGS_H
#define FFTPLOTMANAGERSETTINGS_H

#include <timeplotcomponent.h>

#include <QLabel>

#include <gui/spinbox_a.hpp>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/plotautoscaler.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/toolcomponent.h>
#include <gui/channelcomponent.h>
#include "adcinterfaces.h"

#include "fftplotmanager.h"
#include "menuspinbox.h"

namespace scopy {
namespace adc {

using namespace scopy::gui;

class SCOPY_ADC_EXPORT FFTPlotManagerSettings : public QWidget, public ToolComponent, public Menu
{
	Q_OBJECT
public:
	typedef enum
	{
		XMODE_SAMPLES,
		XMODE_TIME,
		XMODE_OVERRIDE
	} XMode;

	FFTPlotManagerSettings(FFTPlotManager *plot, QWidget *parent = nullptr);
	~FFTPlotManagerSettings();

	double sampleRate() const;
	void setSampleRate(double newSampleRate);

	uint32_t bufferSize() const;
	void setBufferSize(uint32_t newBufferSize);

	MenuWidget *menu() override;

	double freqOffset() const;
	void setFreqOffset(double newFreqOffset);

	bool complexMode() const;
	void setComplexMode(bool newComplexMode);

	void setEnableAddRemovePlot(bool b);

public Q_SLOTS:
	void onStart() override;
	void onStop() override {}
	void onInit() override;
	void onDeinit() override {}

	void addChannel(ChannelComponent *c);
	void removeChannel(ChannelComponent *c);

	void addSampleRateProvider(SampleRateProvider *s);
	void removeSampleRateProvider(SampleRateProvider *s);

	void addPlot(FFTPlotComponent *plt);
	void removePlot(FFTPlotComponent *p);
	void updateXAxis();

Q_SIGNALS:
	void complexModeChanged();
	void sampleRateChanged();
	void freqOffsetChanged();
	void bufferSizeChanged();
	void samplingInfoChanged(SamplingInfo);
	void requestOpenMenu();
private Q_SLOTS:
	double readSampleRate();

private:
	FFTPlotManager *m_plotManager;

	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createXAxisMenu(QWidget *parent = nullptr);
	QWidget *createYAxisMenu(QWidget *parent = nullptr);
	void updateXModeCombo();

	QPen m_pen;
	MenuWidget *m_menu;

	MenuSpinbox *m_bufferSizeSpin;

	MenuSpinbox *m_xmin;
	MenuSpinbox *m_xmax;
	MenuSpinbox *m_sampleRateSpin;
	MenuSpinbox *m_freqOffsetSpin;
	MenuCombo *m_xModeCb;
	MenuSectionWidget *m_plotSection;
	MenuCombo *m_plotCb;
	MapStackedWidget *m_plotStack;

	QPushButton *m_addPlotBtn;
	QMap<uint32_t, QWidget *> m_plotWidgetMap;

	bool m_sampleRateAvailable;

	SamplingInfo m_samplingInfo;

	QList<ChannelComponent *> m_channels;
	QList<SampleRateProvider *> m_sampleRateProviders;

	Q_PROPERTY(bool complexMode READ complexMode WRITE setComplexMode NOTIFY complexModeChanged)
	Q_PROPERTY(double sampleRate READ sampleRate WRITE setSampleRate NOTIFY sampleRateChanged)
	Q_PROPERTY(double freqOffset READ freqOffset WRITE setFreqOffset NOTIFY freqOffsetChanged)
	Q_PROPERTY(uint32_t bufferSize READ bufferSize WRITE setBufferSize NOTIFY bufferSizeChanged FINAL)

	void setPlotComboVisible();
};

} // namespace adc
} // namespace scopy

#endif // FFTPLOTMANAGERSETTINGS_H
