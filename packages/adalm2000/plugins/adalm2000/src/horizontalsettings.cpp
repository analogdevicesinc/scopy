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
 */

#include "horizontalsettings.h"

#include "uihelpers.h"
#include <gui/widgets/menuspinbox.h>

#include <QTabBar>
#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::adalm2000;
using namespace scopy::adalm2000::ui;

namespace {
QWidget *page(QWidget *parent, const QList<QWidget *> &content)
{
	auto *w = new QWidget(parent);
	auto *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 6, 0, 0);
	for(QWidget *c : content) {
		c->setParent(w);
		lay->addWidget(c);
	}
	return w;
}
} // namespace

HorizontalSettings::HorizontalSettings(const sweep::Limits &limits, int divs, Tab tab, QWidget *parent)
	: QWidget(parent)
	, m_limits(limits)
	, m_divs(divs)
{
	auto *lay = new QVBoxLayout(this);
	lay->setContentsMargins(0, 0, 0, 0);
	m_tabs = new QTabBar(this);
	m_tabs->setExpanding(true);
	m_tabs->setUsesScrollButtons(false);
	m_tabs->setDrawBase(false);
	m_tabs->setStyleSheet(QStringLiteral("QTabBar::tab { min-width: 0px; }"));
	m_tabs->addTab(tr("Time base"));
	m_tabs->addTab(tr("Samples"));
	lay->addWidget(m_tabs);

	m_timeBase =
		spin(tr("Time Base"), m_sweep.timeBase(divs), QStringLiteral("s/div"), 0, 1, gui::MenuSpinbox::IS_125);
	m_position = spin(tr("Position"), m_sweep.position(), QStringLiteral("s"), -1, 1, gui::MenuSpinbox::IS_FIXED);
	m_points = spin(tr("Samples"), limits.timeBaseSamples, QStringLiteral("samples"), limits.minSamples,
			limits.maxBuffer, gui::MenuSpinbox::IS_125);
	m_points->setScalingEnabled(false);
	applyTimeBaseRange();
	m_rate = spin(tr("Sample Rate"), m_sweep.sampleRate, QStringLiteral("Hz"), limits.rates.first(),
		      limits.rates.last(), gui::MenuSpinbox::IS_125);
	m_buffer = spin(tr("Buffer Size"), m_sweep.bufferSize, QStringLiteral("samples"), limits.minSamples,
			limits.maxBuffer, gui::MenuSpinbox::IS_125);
	m_window = spin(tr("Plot Window"), m_sweep.plotWindow, QStringLiteral("samples"), limits.minSamples,
			std::max(limits.maxWindow, limits.maxBuffer), gui::MenuSpinbox::IS_125);
	m_delay = spin(tr("Delay"), m_sweep.delay, QStringLiteral("samples"), limits.delayMin, limits.delayMax,
		       gui::MenuSpinbox::IS_FIXED);

	for(gui::MenuSpinbox *s : {m_buffer, m_window, m_delay}) {
		s->setScalingEnabled(false);
	}
	m_rate->setCircleEnabled(true);
	m_delay->setCircleEnabled(true);
	static_cast<gui::IncrementStrategyFixed *>(m_delay->incrementStrategy())->setK(4);
	QWidget *pages[] = {page(this, {m_timeBase, m_position, m_points}),
			    page(this, {m_rate, m_buffer, m_window, m_delay})};
	m_window->setVisible(limits.maxWindow > limits.maxBuffer);
	for(QWidget *p : pages) {
		lay->addWidget(p);
	}
	auto show = [pages](int i) {
		pages[0]->setVisible(i == 0);
		pages[1]->setVisible(i == 1);
	};
	connect(m_tabs, &QTabBar::currentChanged, this, show);
	m_tabs->setCurrentIndex(tab);
	show(tab);

	connect(m_timeBase, &gui::MenuSpinbox::valueChanged, this,
		[this](double v) { request(sweep::fromTimeBase(v, m_sweep.position(), m_divs, m_limits)); });
	connect(m_position, &gui::MenuSpinbox::valueChanged, this,
		[this](double v) { request(sweep::fromTimeBase(m_sweep.timeBase(m_divs), v, m_divs, m_limits)); });
	connect(m_points, &gui::MenuSpinbox::valueChanged, this,
		[this](double v) { setTimeBaseSamples(static_cast<int>(v)); });
	connect(m_rate, &gui::MenuSpinbox::valueChanged, this, [this](double v) {
		sweep::Sweep s = m_sweep;
		s.sampleRate = v;
		request(s);
	});
	connect(m_buffer, &gui::MenuSpinbox::valueChanged, this,
		[this](double v) { request(sweep::withBuffer(m_sweep, static_cast<int>(v), m_limits)); });
	connect(m_window, &gui::MenuSpinbox::valueChanged, this, [this](double v) {
		request(sweep::withWindow(m_sweep, m_sweep.bufferSize, static_cast<int>(v), m_limits));
	});
	connect(m_delay, &gui::MenuSpinbox::valueChanged, this, [this](double v) {
		sweep::Sweep s = m_sweep;
		s.delay = qRound(v);
		request(s);
	});
}

void HorizontalSettings::setSweep(const sweep::Sweep &s)
{
	m_sweep = s;
	m_timeBase->setValueSilent(s.timeBase(m_divs));
	m_position->setValueSilent(s.position());
	m_rate->setValueSilent(s.sampleRate);
	m_buffer->setValueSilent(s.bufferSize);
	m_window->setValueSilent(s.plotWindow);
	m_delay->setMinValue(sweep::delayMin(s, m_limits));
	m_delay->setValueSilent(s.delay);
}

void HorizontalSettings::setDelayEnabled(bool on)
{
	m_position->setEnabled(on);
	m_delay->setEnabled(on);
}

void HorizontalSettings::setTimeBaseSamples(int n)
{
	n = sweep::align4(n, m_limits.minSamples, m_limits.maxBuffer);
	m_points->setValueSilent(n);
	if(n == m_limits.timeBaseSamples) {
		return;
	}
	m_limits.timeBaseSamples = n;
	applyTimeBaseRange();
	Q_EMIT timeBaseSamplesChanged(n);
	request(sweep::fromTimeBase(m_sweep.timeBase(m_divs), m_sweep.position(), m_divs, m_limits));
}

void HorizontalSettings::applyTimeBaseRange()
{
	const double maxTimeBase = (m_limits.streamTimeBase > 0 ? m_limits.maxWindow : m_limits.timeBaseSamples) /
		m_limits.rates.first() / m_divs;
	const double maxPosition = maxTimeBase * m_divs - m_limits.delayMin / m_limits.rates.first();
	m_timeBase->setMinValue(m_limits.minSamples / m_limits.rates.last() / m_divs);
	m_timeBase->setMaxValue(maxTimeBase);
	m_position->setMinValue(-maxPosition);
	m_position->setMaxValue(maxPosition);
}

void HorizontalSettings::request(const sweep::Sweep &s)
{
	Q_EMIT sweepRequested(sweep::normalized(s, m_limits));
	// Shows the sweep back when the tool did not take it.
	setSweep(m_sweep);
}

#include "moc_horizontalsettings.cpp"
