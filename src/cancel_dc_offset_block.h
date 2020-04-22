#ifndef CANCEL_DC_OFFSET_BLOCK_H
#define CANCEL_DC_OFFSET_BLOCK_H

#include "signal_sample.hpp"

#include <gnuradio/hier_block2.h>

namespace adiscope {
class cancel_dc_offset_block : public gr::hier_block2, public QObject
{
public:
	cancel_dc_offset_block(size_t buffer_size, bool enabled);
	~cancel_dc_offset_block();

	void set_enabled(bool enabled);

	void set_buffer_size(size_t buffer_size);

	float get_dc_offset() const;

private:
	bool d_enabled;
	size_t d_buffer_size;
	float d_dc_offset;
	boost::shared_ptr<signal_sample> d_signal;

	void _build_and_connect_blocks();
};
} // namespace adiscope

#endif // CANCEL_DC_OFFSET_BLOCK_H
