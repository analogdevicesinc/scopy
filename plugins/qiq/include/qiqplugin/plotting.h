#ifndef PLOTTING_H
#define PLOTTING_H

#include "plotwidget.h"
#include "scopy-qiqplugin_config.h"
#include "scopy-qiqplugin_export.h"
#include <QFile>
#include <QFutureWatcher>
#include <QTimer>
#include <QWidget>
#include <QDir>

namespace scopy::qiqplugin {
class SCOPY_QIQPLUGIN_EXPORT Plotting : public QWidget
{
	Q_OBJECT
public:
	Plotting(QWidget *parent = nullptr);
	~Plotting();

	bool mapFile(const QString &path);
	void unmapFile();

private:
	struct Sample
	{
		qint16 ch1;
		qint16 ch2;
	};

	QFutureWatcher<void> *m_readFw;
	QFile m_file;
	uchar *m_data;
	qint64 m_size;
	bool m_init = true;
	QPushButton *m_stopBtn;
	QPushButton *m_rqstBtn;
	PlotWidget *m_receiverPlot;
	QTimer m_timer;
	const QString SCRIPT = QString(QIQPLUGIN_RES_PATH) + QDir::separator() + "m2k_acq.py";
	const QString SHARED_FILE = QString(QIQPLUGIN_RES_PATH) + QDir::separator() + "shared_data.bin";
	const QString READ_LOCK = QString(QIQPLUGIN_RES_PATH) + QDir::separator() + "read.lock";
	const QString WRITE_LOCK = QString(QIQPLUGIN_RES_PATH) + QDir::separator() + "write.lock";
	const int SAMPLES = 2048;

	void runPython(const QStringList args);
	void plotData(int maxSamples);
	void addPlotChannel(PlotWidget *plot, const QString &label, const QColor &color);
	void configurePlot(PlotWidget *plot, int yMin, int yMax);
	void setupConnections();
	void setupToolTemplate();
	void setupPlotWidget();
	QPushButton *createButton(const QString &text);
	void setupButtons();
};
} // namespace scopy::qiqplugin
#endif // PLOTTING_H
