/*
 * Copyright (c) 2020 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef PATTERNGENERATOR_H
#define PATTERNGENERATOR_H

#include "m2ktool.hpp"
#include "oscilloscope_plot.hpp"
#include "buffer_previewer.hpp"
#include "gui/spinbox_a.hpp"
#include "mousewheelwidgetguard.h"
#include "gui/genericlogicplotcurve.h"

#include <libm2k/m2k.hpp>
#include <libm2k/contextbuilder.hpp>
#include <libm2k/digital/m2kdigital.hpp>
#include <libm2k/enums.hpp>

#include <QScrollBar>
#include <QQueue>
#include <QTimer>
#include <QMap>

using namespace libm2k;
using namespace libm2k::digital;
using namespace libm2k::context;

namespace Ui {
class PatternGenerator;
}

namespace adiscope {
class BaseMenu;
class CustomPushButton;
}

namespace adiscope::m2k {
class Filter;
class DIOManager;
class PatternUI;

namespace logic {

class PatternGenerator_API;
class PatternGenerator : public M2kTool
{
	Q_OBJECT

	friend class PatternGenerator_API;
public:
	explicit PatternGenerator(struct iio_context *ctx, Filter *filt,
				  ToolMenuEntry *tme, QJSEngine *engine,
				  DIOManager *diom, QWidget *parent);
	~PatternGenerator();
	void setNativeDialogs(bool nativeDialogs) override;

Q_SIGNALS:
	void dataAvailable(uint64_t, uint64_t, uint16_t *data);
	void showTool();

private Q_SLOTS:
	void startStop(bool start);
	void generateBuffer();

	void on_btnChannelSettings_toggled(bool);
	void on_btnSettings_clicked(bool checked);
	void on_btnGeneralSettings_toggled(bool);
	void rightMenuFinished(bool opened);
	void channelSelectedChanged(int chIdx, bool selected);
	void on_btnGroupChannels_toggled(bool checked);
	void patternSelected(const QString& pattern, int ch = -1, const QString &json = {});
	void on_btnOutputMode_toggled(bool);
	void regenerate();
	void readPreferences();

protected:
	uint16_t *m_buffer;

private:
	void setupUi();
	void triggerRightMenuToggle(CustomPushButton *, bool checked);
	void toggleRightMenu(CustomPushButton *, bool checked);
	void settingsPanelUpdate(int id);
	void connectSignalsAndSlots();
	void updateChannelGroupWidget(bool visible);
	void setupPatterns();
	void updateChannelGroupPattern(bool visible);
	void updateGroupsAndPatterns();
	void channelInGroupChangedPosition(int from, int to);
	void channelInGroupRemoved(int position);
	void loadTriggerMenu();
	uint64_t computeSampleRate() const;
	uint64_t computeBufferSize(uint64_t sampleRate) const;
	uint16_t remapBuffer(uint8_t *mapping, uint32_t val);
	void commitBuffer(const QPair<QVector<int>, PatternUI *> &pattern,
			  uint16_t *buffer,
			  uint32_t bufferSize);
	void checkEnabledChannels();
	void removeAnnotationCurveOfPattern(PatternUI *pattern);
	void updateAnnotationCurveChannelsForPattern(const QPair<QVector<int>, PatternUI *> &pattern);

private:
	Ui::PatternGenerator *m_ui;

	CapturePlot m_plot;
	QScrollBar *m_plotScrollBar;

	int m_selectedChannel;
	QVector<GenericLogicPlotCurve *> m_plotCurves;

	QList<CustomPushButton *> m_menuOrder;
	QQueue<QPair<CustomPushButton *, bool>> m_menuButtonActions;

	int m_nbChannels;

	QVector<int> m_currentGroup;
	BaseMenu *m_currentGroupMenu;

	QVector<QPair<QVector<int>, PatternUI *>> m_enabledPatterns;

	M2k* m_m2k_context;
	M2kDigital *m_m2kDigital;
	uint64_t m_bufferSize;
	uint64_t m_sampleRate;

	DIOManager *m_diom;
	uint16_t m_outputMode;

	QTimer *m_singleTimer;

	QMap<PatternUI*, QPair<GenericLogicPlotCurve*, QMetaObject::Connection>> m_annotationCurvePatternUiMap;
};

} // namespace logic
} // namespace adiscope
#endif // PATTERNGENERATOR_H
