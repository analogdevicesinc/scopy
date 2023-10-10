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
