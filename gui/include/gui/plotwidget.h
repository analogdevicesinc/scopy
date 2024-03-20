#ifndef PLOT_H
#define PLOT_H
#include "handles_area.hpp"
#include "plotchannel.h"
#include "scopy-gui_export.h"

#include <QGridLayout>
#include <QWidget>
#include <QwtPlot>
#include <QwtPlotZoomer>

#include <buffer_previewer.hpp>
#include <graticule.h>
#include <symbol_controller.h>

namespace scopy {
class PlotAxis;
class PlotAxisHandle;
class PlotNavigator;
class PlotTracker;

typedef struct _PlotSamplingInfo
{
	_PlotSamplingInfo()
	{
		startingPoint = 0;
		freqOffset = 0;
	}
	uint32_t bufferSize;
	uint32_t plotSize;
	double sampleRate;
	double startingPoint;
	double freqOffset;
} PlotSamplingInfo;

class SCOPY_GUI_EXPORT PlotWidget : public QWidget
{
	Q_OBJECT
public:
	PlotWidget(QWidget *parent = nullptr);
	~PlotWidget();

	void addPlotChannel(PlotChannel *ch);
	void removePlotChannel(PlotChannel *ch);
	QList<PlotChannel *> getChannels();

	QList<PlotAxis *> &plotAxis(int position);
	PlotAxis *xAxis();
	PlotAxis *yAxis();

	void addPlotAxis(PlotAxis *ax);
	// void removePlotAxis(PlotAxis *ax);  - not supported by Qwt

	bool getDisplayGraticule() const;
	void setDisplayGraticule(bool newDisplayGraticule);
	bool eventFilter(QObject *object, QEvent *event) override;

	QwtPlot *plot() const;
	SymbolController *symbolCtrl() const;

	HorizHandlesArea *bottomHandlesArea() const;
	HorizHandlesArea *topHandlesArea() const;
	VertHandlesArea *rightHandlesArea() const;
	VertHandlesArea *leftHandlesArea() const;

	void addPlotAxisHandle(PlotAxisHandle *ax);
	void removePlotAxisHandle(PlotAxisHandle *ax);

	PlotChannel *selectedChannel() const;

	bool showXAxisLabels() const;
	void setShowXAxisLabels(bool newShowXAxisLabels);

	bool showYAxisLabels() const;
	void setShowYAxisLabels(bool newShowYAxisLabels);

	void addPlotInfoSlot(QWidget *w);

	PlotAxis *plotAxisFromId(QwtAxisId axisId);

	PlotNavigator *navigator() const;
	PlotTracker *tracker() const;

	void setUnitsVisible(bool visible);

public Q_SLOTS:
	void replot();
	void selectChannel(PlotChannel *);
	void showAxisLabels();
	void hideAxisLabels();

Q_SIGNALS:
	void canvasSizeChanged();
	// These are only emitted if you set setMouseTracking(true) (not done by default)
	void mouseButtonPress(const QMouseEvent *event);
	void mouseButtonRelease(const QMouseEvent *event);
	void mouseMove(const QMouseEvent *event);
	void channelSelected(PlotChannel *ch);
	void addedChannel(PlotChannel *ch);
	void removedChannel(PlotChannel *ch);

private:
	QwtPlot *m_plot;
	QwtPlotZoomer *m_zoomer;
	QGridLayout *m_layout;

	QList<PlotChannel *> m_plotChannels;
	QList<QwtPlotScaleItem *> m_scaleItems;

	QList<PlotAxis *> m_plotAxis[QwtAxis::AxisPositions];
	QList<PlotAxisHandle *> m_plotAxisHandles[QwtAxis::AxisPositions];

	PlotNavigator *m_navigator;
	PlotTracker *m_tracker;

	PlotAxis *m_xAxis;
	PlotAxis *m_yAxis;

	int m_xPosition;
	int m_yPosition;

	bool displayGraticule;
	Graticule *graticule;

	bool m_showXAxisLabels;
	bool m_showYAxisLabels;

	SymbolController *m_symbolCtrl;
	PlotChannel *m_selectedChannel;

	/* Adjacent areas */
	HorizHandlesArea *m_bottomHandlesArea;
	VertHandlesArea *m_rightHandlesArea;
	HorizHandlesArea *m_topHandlesArea;
	VertHandlesArea *m_leftHandlesArea;

	BufferPreviewer *m_bufferPreviewer;

	void setAxisScalesVisible(bool visible);
	void setupAxisScales();
	void setupOpenGLCanvas();
	void setupHandlesArea();
	void setupNavigator();
	void hideDefaultAxis();
};

} // namespace scopy

#endif // PLOT_H
