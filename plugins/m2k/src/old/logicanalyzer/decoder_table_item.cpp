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
#include "decoder_table_item.hpp"
#include <qwt_painter.h>
#include <qwt_point_mapper.h>
#include <qwt_scale_map.h>
#include <QDebug>
#include <qwt_text.h>

namespace scopy::m2k {


namespace logic {


void DecoderTableItemDelegate::paint(
		QPainter *painter,
		const QStyleOptionViewItem &option,
		const QModelIndex &index
		) const {
	auto data = index.data();
	if (data.canConvert<DecoderTableItem>()) {
		DecoderTableItem decoderItem = qvariant_cast<DecoderTableItem>(data);
		decoderItem.paint(painter, option.rect, option.palette);
	} else {
		QStyledItemDelegate::paint(painter, option, index);
	}
}



QSize DecoderTableItemDelegate::sizeHint(
		const QStyleOptionViewItem &option,
		const QModelIndex &index
		) const {
	if (index.data().canConvert<DecoderTableItem>()) {
		DecoderTableItem decoderItem = qvariant_cast<DecoderTableItem>(index.data());
		return decoderItem.sizeHint();
	}
	return QStyledItemDelegate::sizeHint(option, index);
}


DecoderTableItem::DecoderTableItem(AnnotationCurve *curve, uint64_t start, uint64_t end, QVector<QString> filter, bool flag):
	curve(curve),
	startSample(start),
	endSample(end),
	filteredMessages(filter),
	tableInfoFlag(flag)
{

}

double DecoderTableItem::startTime() const
{
	if (curve != nullptr)
		return curve->fromSampleToTime(startSample);
	return 0;
}

double DecoderTableItem::endTime() const
{
	if (curve != nullptr)
		return curve->fromSampleToTime(endSample);
	return 0;
}

void DecoderTableItem::paint(
		QPainter *painter,
		const QRect &rect,
		const QPalette &palette
		) const {

	if (curve == nullptr){
		return;
	}
	painter->save();

	// Shift to start of table
	painter->translate(rect.x(), rect.y());

	// Set border color
	painter->setPen(QPen(QBrush(Qt::black), 1));

	QwtScaleMap xmap, ymap;
	xmap.setPaintInterval(0, rect.width());
	if (tableInfoFlag) {
		ymap.setPaintInterval(4 + curve->m_infoHeight, rect.height());
	} else {
		ymap.setPaintInterval(4, rect.height());
	}
	// qDebug() << "rect: " << rect << Qt::endl;

	QwtPointMapper mapper;
	mapper.setFlag( QwtPointMapper::RoundPoints, QwtPainter::roundingAlignment( painter ) );
	// mapper.setBoundingRect(rect);  // Seems to have no effect?

	// Scale the annotation to fit in the column
	const double lower = startTime();
	const double upper = endTime();
	double padding = (upper - lower) * 0.05;
	if (padding == 0) {
		padding = 0.0001;
	}

	const QwtInterval interval = QwtInterval(lower-padding, upper+padding);
	xmap.setScaleInterval(interval.minValue(), interval.maxValue());

	// Shift curve offset to 0 and block any signals while doing it since the
	// changes are restored after drawing. Without this the rows are shifted out
	// of place.
	QSignalBlocker signalBlocker(curve);
	const auto originalPixelOffset = curve->getPixelOffset();
	curve->setPixelOffset(0);

	// The title size is reserved for the titles in the normal plot
	// this is unused here.
	const QSize titleSize = QSize(0, 0);

	// Draw all annotations in the sample range
	int offset = 0; // TODO: This does not retain the original order

	if (curve->getAnnotationRows().size() == 0) {
		return;
	}

	if (tableInfoFlag) {
		curve->drawAnnotationInfo(offset, startSample, endSample, painter, xmap, ymap, rect);
	}
	for (int row = 0; row < curve->getAnnotationRows().size(); ++row) {
		auto entry = *std::find_if(curve->getAnnotationRows().begin(), curve->getAnnotationRows().end(),
				       [row](const std::pair<const Row, RowData> &t) -> bool{
			return t.first.index() == row;
		});
		const RowData &data = entry.second;

		if (filteredMessages.contains(curve->fromTitleToRowType(entry.first.title()))) continue;
		if (data.size() == 0) continue;
		vector<Annotation> range;
		if (startSample == 0) {
			data.get_annotation_subset(range, startSample, endSample);
		} else {
			data.get_annotation_subset(range, startSample - 1, endSample);
		}

		for (const auto &ann : range) {
			curve->drawAnnotation(
						offset, ann, painter, xmap, ymap, rect, mapper,
						interval, titleSize);
		}
		offset += 1;
	}

	// Restore
	curve->setPixelOffset(originalPixelOffset);

	painter->restore();
}

QSize DecoderTableItem::sizeHint() const
{
	return itemSize;
}



} // namespace logic
} // namespace scopy

