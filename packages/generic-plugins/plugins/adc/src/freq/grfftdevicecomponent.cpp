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

#include "grfftdevicecomponent.h"
#include "grfftchannelcomponent.h"
#include "fftmenucontrols.h"
#include <widgets/menucollapsesection.h>
#include <widgets/menusectionwidget.h>
#include <widgets/menucombo.h>
#include <widgets/menuspinbox.h>
#include <gui/plotmarkercontroller.h>
#include <style.h>
#include <QBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QVBoxLayout>

using namespace scopy;
using namespace scopy::adc;

GRFFTDeviceComponent::GRFFTDeviceComponent(GRIIODeviceSourceNode *node, QWidget *parent)
	: GRDeviceComponent(node, parent)
{}

static void attachOverrideWarning(MenuSectionCollapseWidget *section)
{
	QAbstractButton *header = section->collapseSection()->header();
	auto *hlay = qobject_cast<QHBoxLayout *>(header->layout());
	if(!hlay)
		return;

	QLabel *icon = new QLabel(header);
	int iconSize = Style::getDimension(json::global::unit_1);
	icon->setPixmap(Style::getPixmap(":/gui/icons/warning.svg", Style::getColor(json::theme::content_default))
				.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
	icon->setToolTip("May be overridden by individual channel settings.");
	icon->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	icon->setContentsMargins(6, 0, 6, 0);

	// Insert right after the title widget (index 0), before the spacer + ctrl.
	hlay->insertWidget(1, icon);
}

QWidget *GRFFTDeviceComponent::buildExtraMenu(QWidget *parent)
{
	QWidget *w = new QWidget(parent);
	auto *lay = new QVBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(10);
	lay->addWidget(createMarkerMenu(w));
	lay->addWidget(createAveragingMenu(w));
	lay->addWidget(createMinMaxHoldMenu(w));
	return w;
}

QWidget *GRFFTDeviceComponent::createMarkerMenu(QWidget *parent)
{
	MarkerMenuControls c = buildMarkerMenu(parent, true);
	// Fixed-marker "editable" toggle is a per-channel concern; hide it here.
	c.fixedEditSwitch->hide();
	attachOverrideWarning(c.section);

	connect(c.section->collapseSection()->header(), &QAbstractButton::toggled, this, [=](bool b) {
		forEachChannel<GRFFTChannelComponent>([=](GRFFTChannelComponent *ch) { ch->applyMarkerEnabled(b); });
		if(b) {
			int t = c.typeCombo->combo()->currentData().toInt();
			forEachChannel<GRFFTChannelComponent>(
				[=](GRFFTChannelComponent *ch) { ch->applyMarkerType(t); });
		}
	});
	connect(c.typeCombo->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int) {
		int t = c.typeCombo->combo()->currentData().toInt();
		forEachChannel<GRFFTChannelComponent>([=](GRFFTChannelComponent *ch) { ch->applyMarkerType(t); });
	});
	connect(c.countSpin, &MenuSpinbox::valueChanged, this, [=](double cnt) {
		int n = static_cast<int>(cnt);
		forEachChannel<GRFFTChannelComponent>([=](GRFFTChannelComponent *ch) { ch->applyMarkerCount(n); });
	});

	return c.section;
}

QWidget *GRFFTDeviceComponent::createAveragingMenu(QWidget *parent)
{
	AveragingMenuControls c = buildAveragingMenu(parent);
	attachOverrideWarning(c.section);

	connect(c.section->collapseSection()->header(), &QAbstractButton::toggled, this, [=](bool b) {
		forEachChannel<GRFFTChannelComponent>([=](GRFFTChannelComponent *ch) { ch->applyAveragingEnabled(b); });
		if(b) {
			int sz = c.sizeSpin->value();
			forEachChannel<GRFFTChannelComponent>(
				[=](GRFFTChannelComponent *ch) { ch->applyAveragingSize(sz); });
		}
	});
	connect(c.sizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int value) {
		forEachChannel<GRFFTChannelComponent>(
			[=](GRFFTChannelComponent *ch) { ch->applyAveragingSize(value); });
	});

	return c.section;
}

QWidget *GRFFTDeviceComponent::createMinMaxHoldMenu(QWidget *parent)
{
	MinMaxHoldMenuControls c = buildMinMaxHoldMenu(parent);
	attachOverrideWarning(c.section);

	connect(c.section->collapseSection()->header(), &QAbstractButton::toggled, this, [=](bool b) {
		forEachChannel<GRFFTChannelComponent>(
			[=](GRFFTChannelComponent *ch) { ch->applyMinMaxHoldEnabled(b); });
	});
	connect(c.minSwitch, &QCheckBox::toggled, this, [=](bool b) {
		forEachChannel<GRFFTChannelComponent>([=](GRFFTChannelComponent *ch) { ch->applyMinHoldEnabled(b); });
	});
	connect(c.maxSwitch, &QCheckBox::toggled, this, [=](bool b) {
		forEachChannel<GRFFTChannelComponent>([=](GRFFTChannelComponent *ch) { ch->applyMaxHoldEnabled(b); });
	});
	connect(c.minReset, &QPushButton::clicked, this, [=]() {
		forEachChannel<GRFFTChannelComponent>([=](GRFFTChannelComponent *ch) { ch->resetMinHold(); });
	});
	connect(c.maxReset, &QPushButton::clicked, this, [=]() {
		forEachChannel<GRFFTChannelComponent>([=](GRFFTChannelComponent *ch) { ch->resetMaxHold(); });
	});

	return c.section;
}
