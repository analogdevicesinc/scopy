#ifndef GRPROXYBLOCK_H
#define GRPROXYBLOCK_H

#include "scopy-gr-util_export.h"

#include <gnuradio/basic_block.h>

#include <QObject>

namespace scopy::grutil {

class GRTopBlock;

class SCOPY_GR_UTIL_EXPORT GRProxyBlock : public QObject
{
	Q_OBJECT
public:
	GRProxyBlock(QObject *parent = nullptr);
	virtual ~GRProxyBlock();

	virtual void build_blks(GRTopBlock *top);
	virtual void destroy_blks(GRTopBlock *top);
	virtual void connect_blk(GRTopBlock *top, GRProxyBlock *src);
	virtual void disconnect_blk(GRTopBlock *top);

	void setEnabled(bool v);
	bool enabled();
	bool built();
	virtual QList<gr::basic_block_sptr> getGrStartPoint();
	virtual gr::basic_block_sptr getGrEndPoint();

Q_SIGNALS:
	void requestRebuild();

protected:
	QList<gr::basic_block_sptr> start_blk; // QList of (?)
	gr::basic_block_sptr end_blk;
	bool m_enabled;
	//	bool m_built;
};

} // namespace scopy::grutil

#endif // GRPROXYBLOCK_H
