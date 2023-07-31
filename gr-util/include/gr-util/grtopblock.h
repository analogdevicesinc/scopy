#ifndef GRTOPBLOCK_H
#define GRTOPBLOCK_H

#include <QObject>
#include <gnuradio/top_block.h>
#include "griiodevicesource.h"
#include "grsignalpath.h"
#include "scopy-gr-util_export.h"

namespace scopy::grutil {
class SCOPY_GR_UTIL_EXPORT GRTopBlock : public QObject
{
	Q_OBJECT
public:
	GRTopBlock(QString name, QObject *parent = nullptr);
	~GRTopBlock();
	void registerSignalPath(GRSignalPath* path);
	void unregisterSignalPath(GRSignalPath* path);
	QList<GRSignalPath*> signalPaths() { return m_signalPaths;}

	void registerIIODeviceSource(GRIIODeviceSource *);
	void unregisterIIODeviceSource(GRIIODeviceSource *);

	void connect(gr::basic_block_sptr src, int srcPort, gr::basic_block_sptr dst, int dstPort);

	gr::top_block_sptr getGrBlock();

Q_SIGNALS:
	void builtSignalPaths();
	void teardownSignalPaths();
	void aboutToStart();
	void started();
	void aboutToStop();
	void stopped();

public Q_SLOTS:
	void build();
	void teardown();
	void rebuild();
	void start();
	void stop();
	void run();

private:
	gr::top_block_sptr top;
	QString m_name;
	bool running;
	bool built;
	QList<GRSignalPath*> m_signalPaths;
	QList<GRIIODeviceSource*> m_iioDeviceSources;
};

}
#endif // GRTOPBLOCK_H
