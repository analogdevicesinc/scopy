#ifndef CANCEL_DC_OFFSET_BLOCK_H
#define CANCEL_DC_OFFSET_BLOCK_H

#include <gnuradio/hier_block2.h>

namespace adiscope {
class cancel_dc_offset_block : public gr::hier_block2
{
public:
        cancel_dc_offset_block(size_t buffer_size, bool enabled);
        ~cancel_dc_offset_block();

        void set_enabled(bool enabled);

private:
        bool d_enabled;
        size_t d_buffer_size;

        void _build_and_connect_blocks();

};
}

#endif // CANCEL_DC_OFFSET_BLOCK_H
