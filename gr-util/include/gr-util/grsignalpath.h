#ifndef GRSIGNALPATH_H
#define GRSIGNALPATH_H
#include "grproxyblock.h"
#include "scopy-gr-util_export.h"

#include <QObject>

namespace scopy::grutil {
class GRTopBlock;
class GRProxyBlock;

class SCOPY_GR_UTIL_EXPORT GRSignalPath : public GRProxyBlock
{
	Q_OBJECT
public:
	GRSignalPath(QString name, QObject *parent = nullptr);
	void append(GRProxyBlock *p);
	void build_blks();
	void destroy_blks();
	virtual QList<gr::basic_block_sptr> getGrStartPoint();
	virtual gr::basic_block_sptr getGrEndPoint();
	virtual void connect_blk(GRTopBlock *top, GRProxyBlock *src);
	virtual void disconnect_blk(GRTopBlock *top);
	QString name() const;
	QList<GRProxyBlock *> path();

protected:
	QList<GRProxyBlock *> list;
	QString m_name;
};
} // namespace scopy::grutil
#endif // GRSIGNALPATH_H
