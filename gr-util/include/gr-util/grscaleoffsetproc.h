#ifndef GRSCALEOFFSETPROC_H
#define GRSCALEOFFSETPROC_H

#include "grproxyblock.h"
#include "scopy-gr-util_export.h"

#include <gnuradio/blocks/add_const_v.h>
#include <gnuradio/blocks/multiply_const.h>

namespace scopy::grutil {
class SCOPY_GR_UTIL_EXPORT GRScaleOffsetProc : public GRProxyBlock
{
public:
	GRScaleOffsetProc(QObject *parent = nullptr);
	void setScale(double sc);
	void setOffset(double off);
	void build_blks(GRTopBlock *top);
	void destroy_blks(GRTopBlock *top);

protected:
	gr::blocks::add_const_v<float>::sptr add;
	gr::blocks::multiply_const_ff::sptr mul;
	double m_scale;
	double m_offset;
	GRTopBlock *m_top;
};
} // namespace scopy::grutil
#endif // GRSCALEOFFSETPROC_H
