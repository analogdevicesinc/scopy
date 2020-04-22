#ifndef MARKER_CONTROLLER_H
#define MARKER_CONTROLLER_H

#include <QList>
#include <QObject>
#include <qwt_plot.h>

#include <memory>

class QwtPlotPicker;

namespace adiscope {

class SpectrumMarker;

class MarkerController : public QObject
{
	Q_OBJECT

public:
	typedef std::shared_ptr<SpectrumMarker> marker_sptr;

	explicit MarkerController(QwtPlot* plot);
	MarkerController(const MarkerController&) = delete;
	void operator=(const MarkerController&) = delete;

	bool enabled() const;
	void setEnabled(bool en);

	void registerMarker(marker_sptr marker);
	void unRegisterMarker(marker_sptr marker);

	marker_sptr selectedMarker() const;
	void markerBringToFront(marker_sptr marker);

	const QwtPlot* plot() const;
	QwtPlot* plot();

Q_SIGNALS:
	void markerSelected(std::shared_ptr<SpectrumMarker>&);
	void markerPositionChanged(std::shared_ptr<SpectrumMarker>&);
	void markerReleased(std::shared_ptr<SpectrumMarker>&);

private Q_SLOTS:
	void onPickerSelected(QPointF);
	void onPickerMoved(QPoint);

private:
	void selectMarker(marker_sptr marker);

private:
	QwtPlotPicker* d_picker;

	QList<marker_sptr> d_markers;
	QList<marker_sptr> d_mrks_overlap_order;
	marker_sptr d_picked_mrk;
	marker_sptr d_selected_mkr;

	bool d_item_moving;
	double d_mrks_default_z;
};

} /* namespace adiscope */

#endif // MARKER_CONTROLLER_H
