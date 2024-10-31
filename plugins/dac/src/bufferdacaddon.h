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

#ifndef BUFFERDACADDON_H_
#define BUFFERDACADDON_H_

#include <toolbuttons.h>
#include <menuonoffswitch.h>
#include <iio-widgets/iiowidget.h>
#include <gui/widgets/menuspinbox.h>

#include <QWidget>
#include <QTextBrowser>

#include "filebrowser.h"
#include "dacaddon.h"

namespace scopy {
namespace dac {
class DacDataModel;
class DataBuffer;
class TxNode;
class BufferDacAddon : public DacAddon
{
	Q_OBJECT
public:
	BufferDacAddon(DacDataModel *model, QWidget *parent = nullptr);
	virtual ~BufferDacAddon();
	virtual void enable(bool enable);
	virtual void setRunning(bool running);

Q_SIGNALS:
	void log(QString log);
	void toggleCyclic(bool toggled);

public Q_SLOTS:
	void dataReload();
	void onLoadFailed();
	void onLoadFinished();

private Q_SLOTS:
	void load(QString path);
	void updateGuiStrategyWidget();
	void forwardSamplingFrequencyChange(QDateTime timestamp, QString oldData, QString newData, int retCode,
					    bool readOp);

private:
	DacDataModel *m_model;
	QWidget *m_optionalGuiStrategy;
	DataBuffer *m_dataBuffer;
	RunBtn *m_runBtn;
	gui::MenuSpinbox *m_bufferSizeSpin;
	gui::MenuSpinbox *m_fileSizeSpin;
	gui::MenuSpinbox *m_kernelCountSpin;
	MenuOnOffSwitch *m_cyclicBtn;
	QTextBrowser *m_logText;
	FileBrowser *fm;

	void runBtnToggled(bool toggled);
	QWidget *createMenu(TxNode *node);
	QWidget *createAttrMenu(TxNode *node, QWidget *parent);
	void detectSamplingFrequency(IIOWidget *w);
	void enableFirstChannels(int channelCount);
};
} // namespace dac
} // namespace scopy

#endif // BUFFERDACADDON_H_
