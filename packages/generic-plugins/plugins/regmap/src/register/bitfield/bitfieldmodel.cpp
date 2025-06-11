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

#include "bitfieldmodel.hpp"

using namespace scopy::regmap;

BitFieldModel::BitFieldModel(QString name, int width, int regOffset, QString description, QObject *parent)
	: name(name)
	, width(width)
	, regOffset(regOffset)
	, description(description)
	, QObject{parent}
{
	if(description == "Reserved") {
		reserved = true;
	}

	defaultValue = 0;
}
BitFieldModel::BitFieldModel(QString name, QString access, int defaultValue, QString description, QString visibility,
			     int width, QString notes, int bitOffset, int regOffset, int sliceWidth,
			     QVector<BitFieldOption *> *options, QObject *parent)
	: name(name)
	, access(access)
	, defaultValue(defaultValue)
	, description(description)
	, visibility(visibility)
	, width(width)
	, notes(notes)
	, bitOffset(bitOffset)
	, regOffset(regOffset)
	, sliceWidth(sliceWidth)
	, options(options)
	, QObject{parent}
{
	reserved = false;
}

BitFieldModel::~BitFieldModel()
{
	if(options)
		delete options;
}
QString BitFieldModel::getName() const { return name; }

QString BitFieldModel::getAccess() const { return access; }

int BitFieldModel::getDefaultValue() const { return defaultValue; }

QString BitFieldModel::getDescription() const { return description; }

QString BitFieldModel::getVisibility() const { return visibility; }

int BitFieldModel::getWidth() const { return width; }

QString BitFieldModel::getNotes() const { return notes; }

int BitFieldModel::getBitOffset() const { return bitOffset; }

int BitFieldModel::getRegOffset() const { return regOffset; }

int BitFieldModel::getSliceWidth() const { return sliceWidth; }

QVector<BitFieldOption *> *BitFieldModel::getOptions() const { return options; }
