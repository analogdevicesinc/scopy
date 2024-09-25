#ifndef GRTOPBLOCK_H
#define GRTOPBLOCK_H

#include "griiodevicesource.h"
#include "grsignalpath.h"
#include "scopy-gr-util_export.h"

#include <gnuradio/top_block.h>

#include <QObject>

namespace scopy::grutil {
class SCOPY_GR_UTIL_EXPORT GRTopBlock : public QObject
{
	Q_OBJECT
public:
	GRTopBlock(QString name, QObject *parent = nullptr);
	~GRTopBlock();
	void registerSignalPath(GRSignalPath *path);
	void unregisterSignalPath(GRSignalPath *path);
	QList<GRSignalPath *> signalPaths() { return m_signalPaths; }

	void registerIIODeviceSource(GRIIODeviceSource *);
	void unregisterIIODeviceSource(GRIIODeviceSource *);

	void setVLen(size_t vlen);
	size_t vlen();

	void connect(gr::basic_block_sptr src, int srcPort, gr::basic_block_sptr dst, int dstPort);

	gr::top_block_sptr getGrBlock();

	QString name() const;

Q_SIGNALS:
	void aboutToBuild();
	void builtSignalPaths();
	void aboutToTeardown();
	void teardownSignalPaths();
	void aboutToStart();
	void started();
	void aboutToStop();
	void stopped();
	void finished();
	void requestRebuild();

public Q_SLOTS:
	void build();
	void teardown();
	void rebuild();
	void start();
	void stop();
	void run();
	void suspendBuild();
	void unsuspendBuild();

private:
	bool m_suspended;
	gr::top_block_sptr top;
	QString m_name;
	bool running;
	bool built;
	size_t m_vlen;
	QList<GRSignalPath *> m_signalPaths;
	QList<GRIIODeviceSource *> m_iioDeviceSources;
};

} // namespace scopy::grutil
#endif // GRTOPBLOCK_H
