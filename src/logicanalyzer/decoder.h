#ifndef DECODER_H
#define DECODER_H

#include <map>
#include <memory>
#include <set>
#include <vector>

#include <glib.h>
#include <QString>
#include <memory>

#include "annotation.h"
#include "row.h"
#include "rowdata.h"

using std::map;
using std::string;
using std::vector;
using std::shared_ptr;

struct srd_decoder;
struct srd_decoder_inst;
struct srd_channel;
struct srd_session;

namespace adiscope {
namespace logic {
class Decoder;
}
}

struct DecodeChannel
{
    uint16_t id;     ///< Global numerical ID for the decode channels in the stack
    uint16_t bit_id; ///< Tells which bit within a sample represents this channel
    const bool is_optional;
    bool assigned_signal;
    const QString name, desc;
    int initial_pin_state;
	const shared_ptr<adiscope::logic::Decoder> decoder_;
    const srd_channel *pdch_;
};

struct DecodeSegment
{
    map<const Row, RowData> annotation_rows;
//    pv::util::Timestamp start_time;
    double samplerate;
    int64_t samples_decoded_incl, samples_decoded_excl;
};

namespace adiscope {
namespace logic {

class Decoder
{
public:
    Decoder(const srd_decoder *const dec);

    virtual ~Decoder();

    const srd_decoder* decoder() const;

    bool shown() const;
    void show(bool show = true);

    const vector<DecodeChannel*>& channels() const;
    void set_channels(vector<DecodeChannel*> channels);

    const map<string, GVariant*>& options() const;

    void set_option(const char *id, GVariant *value);

    void apply_all_options();

    bool have_required_channels() const;

    srd_decoder_inst* create_decoder_inst(srd_session *session);
    void invalidate_decoder_inst();

private:
    const srd_decoder *const decoder_;

    bool shown_;

    vector<DecodeChannel*> channels_;
    map<string, GVariant*> options_;
    srd_decoder_inst *decoder_inst_;
};

}
}
#endif // DECODER_H
