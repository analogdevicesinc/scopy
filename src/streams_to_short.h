#ifndef INCLUDED_BLOCKS_STREAMS_TO_SHORT_H
#define INCLUDED_BLOCKS_STREAMS_TO_SHORT_H

#include <gnuradio/blocks/api.h>
#include <gnuradio/sync_block.h>

namespace adiscope {

class streams_to_short : virtual public gr::sync_block
{
public:
	// gr::blocks::streams_to_vector::sptr
	typedef boost::shared_ptr<streams_to_short> sptr;

	/*!
	 * Make a stream-to-vector block.
	 *
	 * \param itemsize the item size of the stream
	 * \param nstreams number of streams to combine into a vector (vector
	 * size)
	 */
	static sptr make(size_t itemsize, size_t nstreams);
};

} /* namespace adiscope */
#endif /* INCLUDED_BLOCKS_STREAMS_TO_SHORT_H */
