#ifndef QIQINSTRUMENT_H
#define QIQINSTRUMENT_H

#include "scopy-qiqplugin_export.h"
#include <QWidget>
#include <inputconfig.h>
#include <outputinfo.h>
#include <outputconfig.h>
#include <plotmanager.h>
#include <settingsmenu.h>
#include <toolbuttons.h>
#include <QProcess>
#include <qiqcontroller/runresults.h>

namespace scopy::qiqplugin {
class SCOPY_QIQPLUGIN_EXPORT QIQInstrument : public QWidget
{
	Q_OBJECT
public:
	QIQInstrument(QWidget *parent = nullptr);
	~QIQInstrument();

	void setAvailableChannels(QMap<QString, QStringList> channels);
Q_SIGNALS:
	void bufferParamsChanged(const BufferParams &params);
	void requestAnalysisInfo(const QString &type);
	void outputConfigured(const OutputConfig &outConfig);
	void analysisConfigChanged(const QString &type, const QVariantMap &config);
	void bufferDataReady(QVector<QVector<double>> &inputData);
	void runPressed(bool en);
	void requestNewData();

public Q_SLOTS:
	void onAnalysisTypes(const QStringList &types);
	void onInputFormatChanged(const InputConfig &inConfig);
	void onOutputConfig(const OutputConfig &outConfig);
	void onRunResponse(const RunResults &runResults);
	void onAnalysisInfo(const QString &type, const QVariantMap &params, const OutputInfo &outputInfo,
			    const QList<QIQPlotInfo> plotInfoList);
	void onAnalysisConfigured(const QString &type, const QVariantMap &config, const OutputInfo &outputInfo);
	// void onProcessDataCompleted(const RunResults &result);

private:
	void addPlots();
	void removePlots();
	void setupConnections();
	QWidget *createCentralWidget(QWidget *parent = nullptr);
	void createInputPlot();
	void updateXAxis(int samples, int sampleRate);
	void addPlotChannel(const QString &label, const QColor &color);
	void removePlotChannels();
	void updateChannels(int chnlCount);

	RunBtn *m_runBtn;
	PlotManager *m_plotManager;
	SettingsMenu *m_settings;
	QGridLayout *m_plotsLay;
	PlotWidget *m_inputPlot = nullptr;
	QVector<double> m_xAxis;
};
} // namespace scopy::qiqplugin
#endif // QIQINSTRUMENT_H
