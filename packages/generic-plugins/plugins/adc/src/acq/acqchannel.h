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

#ifndef ACQCHANNEL_H
#define ACQCHANNEL_H

#include "acqchannelrepr.h"

#include <core/acq_engine/DataKey.h>

#include <QColor>
#include <QList>
#include <QObject>
#include <QPointer>
#include <QString>

#include <memory>

namespace scopy {
class InstrumentTemplate;

namespace acq {
class DataStore;
}

namespace adc {

class AcqPlotRow;

// A DataKey, one representation of it, and the identity to edit both.
//
// That is the whole of a channel here. There is no FFT, no scale/offset, no YMode:
// those are engine blocks, and a channel that wants a derived stream simply points
// at the key the block writes (pluto_iq_fft rather than pluto_voltage0_raw).
//
// Key and repr are both fixed for the channel's lifetime. Changing either means
// removing this channel and adding a new one — which is what lets the settings page
// be built once and never rebuilt (gui/include/gui/instrumenttemplate.h:206).
class AcqChannel : public QObject
{
	Q_OBJECT
public:
	// `uid` comes from the manager and is the identity behind both the depth
	// claimant and the menu page id. Neither may be derived from the key or the
	// name — see claimant() below.
	AcqChannel(scopy::acq::DataStore *store, const scopy::acq::DataKey &key, const QString &name,
		   const QColor &color, std::unique_ptr<AcqChannelRepr> repr, int uid, QObject *parent = nullptr);
	~AcqChannel() override;

	const scopy::acq::DataKey &key() const { return m_key; }
	QString name() const { return m_name; }
	QColor color() const { return m_color; }
	bool isEnabled() const { return m_enabled; }
	int uid() const { return m_uid; }
	AcqChannelRepr *repr() const { return m_repr.get(); }

	// The DataStore claimant, "acqch-<uid>". Stable for the channel's life, and
	// deliberately not derived from the key or the name:
	//
	//   - claims are keyed (key, claimant) and claimDepth() *replaces* that
	//     claimant's previous claim, so two channels on one key sharing a claimant
	//     string would silently clobber each other;
	//   - names are user-editable, and a name-derived claimant orphans its old claim
	//     on rename, pinning memory at the high-water mark with no way to release it.
	const QString &claimant() const { return m_claimant; }

	// The InstrumentTemplate menu page / rail row id, "acqch:<uid>".
	QString menuId() const;

	void attach(AcqPlotRow *row);
	void detach();

	// The row this channel is attached to, or null once detached. The manager needs it
	// to decide whether removing this channel also removes a row — an exclusive row
	// exists only for the repr that brought its own plot.
	AcqPlotRow *row() const { return m_row; }

	// One cycle. A no-op while disabled: a disabled channel must not pay for a
	// window copy, and its visual is already detached.
	void pull(int plotSize);
	void reset();

	// (Re)register the depth claim, on this channel's key and on every key the repr
	// names in extraKeys() (a curve's X stream). Idempotent — claimDepth() replaces
	// rather than accumulates, so no release is needed first. Correct even when the key
	// does not exist yet: DataStore::write() applies pending claims before the first
	// push, so claiming early is what makes the *first* window full-depth instead of one
	// chunk.
	//
	// Also releases any extra key claimed by a previous call and no longer read, so
	// retargeting the X key does not leave the old one pinned.
	void reclaimDepth(int plotSize, std::size_t bufferSize);

	// A generic CHANNEL section (name, colour, key readout) stacked over the repr's
	// own section, the way Block::withBaseSettings() composes
	// (core/src/acq_engine/Block.cpp:59-70). Built once; the caller hands it to
	// InstrumentTemplate::addMenuPage.
	QWidget *createSettingsPage(InstrumentTemplate *it, QWidget *parent = nullptr);

public Q_SLOTS:
	void setEnabled(bool en);
	void setName(const QString &n);
	void setColor(const QColor &c);

Q_SIGNALS:
	void enabledChanged(bool);
	void nameChanged(QString);
	void colorChanged(QColor);

	// The Delete button on this channel's own settings page was pressed. A signal
	// rather than a direct call because removal is the manager's: the rail row, the menu
	// page and the channel list are all its state, and the page cannot outlive the
	// channel that owns the button. The manager's handler is queued, so the button
	// survives its own click handler.
	void removeRequested();

	// The repr's read requirement changed (e.g. a waterfall's row count), so the
	// manager must re-claim for this channel.
	void depthNeedsReclaim();

private:
	QPointer<scopy::acq::DataStore> m_store;
	const scopy::acq::DataKey m_key;
	QString m_name;
	QColor m_color;

	const int m_uid;
	const QString m_claimant;

	std::unique_ptr<AcqChannelRepr> m_repr;
	AcqPlotRow *m_row{nullptr}; // not owned

	// What extraKeys() returned on the last reclaimDepth(), so the next one can release
	// whatever dropped out of the list. The destructor needs no equivalent:
	// releaseClaimant() sweeps every key at once.
	QList<scopy::acq::DataKey> m_claimedExtra;

	bool m_enabled{true};
};

} // namespace adc
} // namespace scopy

#endif // ACQCHANNEL_H
