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

#include <QWidget>
#include <qwt_point_mapper.h>

#include <libsigrokdecode/libsigrokdecode.h>
#include <memory>
#include <mutex>
#include <scopy/gui/logicanalyzer/annotation.hpp>
#include <scopy/gui/logicanalyzer/generic_logic_plot_curve.hpp>
#include <scopy/gui/logicanalyzer/row.hpp>
#include <scopy/gui/logicanalyzer/row_data.hpp>

namespace scopy {
namespace logic {
class LogicTool;
class Decoder;
} // namespace logic
} // namespace scopy

namespace scopy {
namespace bind {
class Decoder;
}
} // namespace scopy

namespace scopy {
namespace gui {

class AnnotationDecoder;

class AnnotationCurve : public GenericLogicPlotCurve
{
	Q_OBJECT
public:
	AnnotationCurve(logic::LogicTool* logic, std::shared_ptr<logic::Decoder> initialDecoder);
	~AnnotationCurve();

Q_SIGNALS:
	void decoderMenuChanged();

public:
	static void annotationCallback(srd_proto_data* pdata, void* annotationCurve);

	virtual void dataAvailable(uint64_t from, uint64_t to) override;

	void setClassRows(const std::map<std::pair<const srd_decoder*, int>, Row>& classRows);
	void setAnnotationRows(const std::map<Row, RowData>& annotationRows);

	void sortRows();

	void newAnnotations();

	virtual void reset() override;

	QWidget* getCurrentDecoderStackMenu();
	void stackDecoder(std::shared_ptr<scopy::logic::Decoder> decoder);
	std::vector<std::shared_ptr<scopy::logic::Decoder>> getDecoderStack();

	int getVisibleRows() const;

	AnnotationDecoder* getAnnotationDecoder();
	std::vector<std::shared_ptr<scopy::bind::Decoder>> getDecoderBindings();

protected:
	void drawLines(QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap, const QRectF& canvasRect,
		       int from, int to) const override;

private:
	void fillCurve(int row, uint32_t annClass, QPainter* painter, const QwtScaleMap& xMap, const QwtScaleMap& yMap,
		       const QRectF& canvasRect, QPolygonF& polygon) const;

	void closePolyline(int row, uint32_t annClass, QPainter* painter, const QwtScaleMap& xMap,
			   const QwtScaleMap& yMap, QPolygonF& polygon) const;

	void drawTwoSampleAnnotation(int row, const Annotation& ann, QPainter* painter, const QwtScaleMap& xMap,
				     const QwtScaleMap& yMap, const QRectF& canvasRect, const QwtPointMapper& mapper,
				     const QSizeF& titleSize) const;

	void drawOneSampleAnnotation(int row, const Annotation& ann, QPainter* painter, const QwtScaleMap& xMap,
				     const QwtScaleMap& yMap, const QRectF& canvasRect, const QwtPointMapper& mapper,
				     const QSizeF& titleSize) const;

	void drawAnnotation(int row, const Annotation& ann, QPainter* painter, const QwtScaleMap& xMap,
			    const QwtScaleMap& yMap, const QRectF& canvasRect, const QwtPointMapper& mapper,
			    const QSizeF& titleSize) const;

	void drawBlock(int row, uint64_t start, uint64_t end, QPainter* painter, const QwtScaleMap& xMap,
		       const QwtScaleMap& yMap, const QRectF& canvasRect, const QwtPointMapper& mapper) const;

private:
	AnnotationDecoder* m_annotationDecoder;
	mutable std::mutex m_mutex;

	std::map<std::pair<const srd_decoder*, int>, Row> m_classRows;
	std::map<Row, RowData> m_annotationRows;

	std::vector<std::shared_ptr<scopy::bind::Decoder>> m_bindings;

	mutable int m_visibleRows;
};
} // namespace gui
} // namespace scopy

#endif // ANNOTATIONCURVE_H
