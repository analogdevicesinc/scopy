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
 *
 */

#ifndef ATTRIBUTEMONITOR_H
#define ATTRIBUTEMONITOR_H

#include "scopy-dcdebugger_export.h"

#include <QElapsedTimer>
#include <QHash>
#include <QObject>
#include <QPointF>
#include <QPointer>
#include <QStringList>
#include <QVector>

class QTimer;

namespace scopy::component {
class Attribute;
}

namespace scopy::dcdebugger {

// Polls numeric attributes on a timer and keeps a time history of each. A source named like an
// IIO raw value can carry scale / offset attributes: with scaled units the value is
// (raw + offset) * scale. One read per item is in flight at a time; a slow attribute simply
// gets fewer samples.
class SCOPY_DCDEBUGGER_EXPORT AttributeMonitor : public QObject
{
	Q_OBJECT
public:
	struct Stats
	{
		double value = 0;
		double min = 0;
		double max = 0;
		double mean = 0;
		int count = 0;
	};

	explicit AttributeMonitor(QObject *parent = nullptr);
	~AttributeMonitor() override;

	void addItem(const QString &key, component::Attribute *source, component::Attribute *scale = nullptr,
		     component::Attribute *offset = nullptr);
	// Changes what an item polls; its history restarts.
	void setSource(const QString &key, component::Attribute *source, component::Attribute *scale = nullptr,
		       component::Attribute *offset = nullptr);
	QStringList keys() const { return m_order; }
	component::Attribute *source(const QString &key) const;

	void setEnabled(const QString &key, bool enabled);
	bool isEnabled(const QString &key) const;
	QStringList enabledKeys() const;

	int period() const { return m_periodMs; }
	void setPeriod(int ms);
	double history() const { return m_historyS; }
	void setHistory(double seconds);
	bool scaled() const { return m_scaled; }
	void setScaled(bool scaled);

	bool isRunning() const;
	// Seconds since start().
	double now() const;
	// (time, value) samples of the last history() seconds.
	QVector<QPointF> samples(const QString &key) const;
	Stats stats(const QString &key) const;
	QString lastError(const QString &key) const;

	// Parses the leading number of an attribute value ("25.13", "71.000000 dB"); false if none.
	static bool parseValue(const QString &text, double &value);

public Q_SLOTS:
	void start();
	void stop();
	void clear();

Q_SIGNALS:
	void runningChanged(bool running);
	void sampled(const QString &key);
	// After every poll round was issued (redraw hook).
	void ticked();
	void readFailed(const QString &key, const QString &message);

private:
	struct Item
	{
		QPointer<component::Attribute> source;
		QPointer<component::Attribute> scale;
		QPointer<component::Attribute> offset;
		bool enabled = false;
		bool inFlight = false;
		QVector<QPointF> history;
		QString error;
	};

	void tick();
	void poll(const QString &key);
	void refreshScale(const Item &item);
	void trim(Item &item, double now);
	double scaledValue(const Item &item, double raw) const;

	QHash<QString, Item> m_items;
	QStringList m_order;
	QTimer *m_timer;
	QElapsedTimer m_clock;
	int m_periodMs = 100;
	double m_historyS = 30;
	bool m_scaled = true;
};

} // namespace scopy::dcdebugger

#endif // ATTRIBUTEMONITOR_H
