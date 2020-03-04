#include "annotation.h"

extern "C" {
#include <libsigrokdecode/libsigrokdecode.h>
}

#include <vector>

Annotation::Annotation(const srd_proto_data *const pdata, const Row *row) :
    start_sample_(pdata->start_sample),
    end_sample_(pdata->end_sample),
    row_(row)
{
    assert(pdata);
    const srd_proto_data_annotation *const pda =
        (const srd_proto_data_annotation*)pdata->data;
    assert(pda);

    ann_class_ = (Class)(pda->ann_class);

    const char *const *annotations = (char**)pda->ann_text;
    while (*annotations) {
        annotations_.push_back(QString::fromUtf8(*annotations));
        annotations++;
    }
}

uint64_t Annotation::start_sample() const
{
    return start_sample_;
}

uint64_t Annotation::end_sample() const
{
    return end_sample_;
}

Annotation::Class Annotation::ann_class() const
{
    return ann_class_;
}

const vector<QString>& Annotation::annotations() const
{
    return annotations_;
}

const Row* Annotation::row() const
{
    return row_;
}

bool Annotation::operator<(const Annotation &other) const
{
    return (start_sample_ < other.start_sample_);
}
