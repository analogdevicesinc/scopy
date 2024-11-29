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

#ifndef ANNOTATIONCURVE_H
#define ANNOTATIONCURVE_H

#include "annotation.h"
#include "genericlogicplotcurve.h"
#include "row.h"
#include "rowdata.h"
#include "scopy-sigrok-gui_export.h"

#include <QWidget>
#include <qwt_point_mapper.h>

#include <libsigrokdecode/libsigrokdecode.h>
#include <memory>
#include <mutex>

namespace scopy {
namespace logic {
class Decoder;
}
} // namespace scopy

namespace scopy {
namespace bind {
class Decoder;
}
} // namespace scopy

namespace scopy {

class AnnotationDecoder;

struct AnnotationQueryResult
{
	uint64_t index;
	const Annotation *ann;
	inline bool isValid() const { return ann != nullptr; }
};

class SCOPY_SIGROK_GUI_EXPORT AnnotationCurve : public GenericLogicPlotCurve
{
	Q_OBJECT
public:
	AnnotationCurve(std::shared_ptr<logic::Decoder> initialDecoder);
	~AnnotationCurve();

Q_SIGNALS:
	void decoderMenuChanged();

	// Emitted when an annotation is clicked
	void annotationClicked(AnnotationQueryResult result);

public:
	static void annotationCallback(srd_proto_data *pdata, void *annotationCurve);

	virtual void dataAvailable(uint64_t from, uint64_t to, uint16_t *data) override;

	void setClassRows(const std::map<std::pair<const srd_decoder *, int>, Row> &classRows);
	void setAnnotationRows(const std::map<Row, RowData> &annotationRows);
	const std::map<Row, RowData> &getAnnotationRows() const;

	void sort_rows();

	uint64_t getMaxAnnotationCount(int index = -1);

	void newAnnotations();

	virtual void reset() override;

	QWidget *getCurrentDecoderStackMenu();
	void stackDecoder(std::shared_ptr<scopy::logic::Decoder> decoder);
	std::vector<std::shared_ptr<scopy::logic::Decoder>> getDecoderStack();

	int getVisibleRows() const override;
	double getHeightOffset() const override;

	AnnotationDecoder *getAnnotationDecoder();
	std::vector<std::shared_ptr<scopy::bind::Decoder>> getDecoderBindings();

	// Get the annotation at the given point
	AnnotationQueryResult annotationAt(const QPointF &p) const;
	bool testHit(const QPointF &p) const override;

	void drawAnnotation(int row, const Annotation &ann, QPainter *painter, const QwtScaleMap &xMap,
			    const QwtScaleMap &yMap, const QRectF &canvasRect, const QwtPointMapper &mapper,
			    const QwtInterval &interval, const QSizeF &titleSize) const;

	void drawBlock(int row, uint64_t start, uint64_t end, QPainter *painter, const QwtScaleMap &xMap,
		       const QwtScaleMap &yMap, const QRectF &canvasRect, const QwtPointMapper &mapper) const;

	void drawAnnotationInfo(int row, uint64_t start, uint64_t end, QPainter *painter, const QwtScaleMap &xMap,
				const QwtScaleMap &yMap, const QRectF &canvasRect) const;

	const double m_infoHeight = 17 * 2;

	int getState();

	QString fromTitleToRowType(QString title) const;

	void setState(int st);

protected:
	void drawLines(QPainter *painter, const QwtScaleMap &xMap, const QwtScaleMap &yMap, const QRectF &canvasRect,
		       int from, int to) const override;

private:
	void fillAnnotationCurve(int row, uint32_t annClass, QPainter *painter, const QwtScaleMap &xMap,
				 const QwtScaleMap &yMap, const QRectF &canvasRect, QPolygonF &polygon) const;

	void closePolyline(int row, uint32_t annClass, QPainter *painter, const QwtScaleMap &xMap,
			   const QwtScaleMap &yMap, QPolygonF &polygon) const;

	void drawTwoSampleAnnotation(int row, const Annotation &ann, QPainter *painter, const QwtScaleMap &xMap,
				     const QwtScaleMap &yMap, const QRectF &canvasRect, const QwtPointMapper &mapper,
				     const QwtInterval &interval, const QSizeF &titleSize) const;

	void drawOneSampleAnnotation(int row, const Annotation &ann, QPainter *painter, const QwtScaleMap &xMap,
				     const QwtScaleMap &yMap, const QRectF &canvasRect, const QwtPointMapper &mapper,
				     const QwtInterval &interval, const QSizeF &titleSize) const;

	QString formatSeconds(double time) const;

private:
	AnnotationDecoder *m_annotationDecoder;
	mutable std::mutex m_mutex;

	std::map<std::pair<const srd_decoder *, int>, Row> m_classRows;
	std::map<Row, RowData> m_annotationRows;

	std::vector<std::shared_ptr<scopy::bind::Decoder>> m_bindings;

	mutable int m_visibleRows;
	mutable int state = -1;
};
} // namespace scopy

#endif // ANNOTATIONCURVE_H
