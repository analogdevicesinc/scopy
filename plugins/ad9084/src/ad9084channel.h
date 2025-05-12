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

#ifndef AD9084CHANNEL_H
#define AD9084CHANNEL_H

#include "scopy-ad9084_export.h"
#include <iiowidget.h>
#include <iiowidgetgroup.h>
#include <QWidget>
#include <iio.h>

namespace scopy {
namespace ad9084 {

enum SharedAttrCategory
{
	ADC_FREQUENCY = 0,
	DAC_FREQUENCY = 1
};

class SCOPY_AD9084_EXPORT Ad9084Channel : public QWidget
{
	Q_OBJECT
public:
	Ad9084Channel(struct iio_channel *chn, unsigned int chnIdx, QWidget *parent = nullptr);
	~Ad9084Channel();

	bool isInput() const;
	QString label() const;

	void init();
	void addGroup(SharedAttrCategory attr, IIOWidgetGroup *grp);

public Q_SLOTS:
	void readChannel();

Q_SIGNALS:
	void requestEnableUi(bool);

private:
	QString phaseUItoDS(QString data);
	QString phaseDStoUI(QString data);
	QString frequencyDStoUI(QString data);
	QString frequencyUItoDS(QString data);
	void setupFrequency();
	void setupChannelNco(QLayout *lay);
	void setupMainNco(QLayout *lay);
	void setupInChannelsAttrs(QLayout *lay);
	void setupChannelTone(QLayout *lay);
	void setupMainTone(QLayout *lay);

private:
	struct iio_channel *m_channel;
	struct iio_device *m_device;
	QString m_channelLabel;
	QLabel *m_titleLbl;
	bool m_input;
	unsigned int m_chnIdx;
	IIOWidget *m_frequencyWidget;
	MenuSectionWidget *m_chnSection;
	QList<IIOWidget *> m_iioWidgets;
	QMap<SharedAttrCategory, IIOWidgetGroup *> m_iioWidgetGroupList;
};
} // namespace ad9084
} // namespace scopy
#endif // AD9084CHANNEL_H
