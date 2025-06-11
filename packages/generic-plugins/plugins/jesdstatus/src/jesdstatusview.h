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

#ifndef JESDSTATUSVIEW_H
#define JESDSTATUSVIEW_H

#include <QWidget>
#include <QTextEdit>
#include <QLabel>
#include <QThread>
#include <gui/widgets/menusectionwidget.h>
#include "scopy-jesdstatus_export.h"
#include "jesdstatusparser.h"

namespace scopy {
namespace jesdstatus {
class SCOPY_JESDSTATUS_EXPORT JesdStatusView : public QWidget
{
	Q_OBJECT
public:
	JesdStatusView(struct iio_device *dev, QWidget *parent = nullptr);
	virtual ~JesdStatusView();

public Q_SLOTS:
	void update();

private Q_SLOTS:
	void updateUi();

private:
	void updateStatus();
	void updateLaneStatus();
	void appendToStatusLabels(QString lbl, std::function<QPair<QString, VISUAL_STATUS>()> cb,
				  MenuSectionWidget *labelContainer, MenuSectionWidget *valueContainer);
	void appendToLaneValues(unsigned int laneIdx, std::function<QPair<QString, VISUAL_STATUS>(unsigned int)> cb,
				MenuSectionWidget *valueContainer);
	void initLaneStatusValues(QWidget *laneContainer);
	void initStatusValues(QWidget *statusContainer);

	JesdStatusParser *m_parser;
	QThread *m_parserThread;
	QMap<VISUAL_STATUS, QString> m_colorMap;
	typedef QPair<QLabel *, std::function<QPair<QString, VISUAL_STATUS>()>> statusCallback;
	typedef QPair<QLabel *, std::function<QPair<QString, VISUAL_STATUS>(unsigned int)>> laneStatusCallback;

	QMap<QLabel *, statusCallback> m_statusLabels;
	QMap<unsigned int, QVector<laneStatusCallback>> m_laneLabels;
};
} // namespace jesdstatus
} // namespace scopy
#endif // JESDSTATUSVIEW_H
