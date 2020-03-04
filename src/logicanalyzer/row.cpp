#include "row.h"

#include <libsigrokdecode/libsigrokdecode.h>

Row::Row() :
    decoder_(nullptr),
    row_(nullptr)
{
}

Row::Row(int index, const srd_decoder *decoder, const srd_decoder_annotation_row *row) :
    index_(index),
    decoder_(decoder),
    row_(row)
{
}

const srd_decoder* Row::decoder() const
{
    return decoder_;
}

const srd_decoder_annotation_row* Row::row() const
{
    return row_;
}

const QString Row::title() const
{
    if (decoder_ && decoder_->name && row_ && row_->desc)
        return QString("%1: %2")
            .arg(QString::fromUtf8(decoder_->name),
                 QString::fromUtf8(row_->desc));
    if (decoder_ && decoder_->name)
        return QString::fromUtf8(decoder_->name);
    if (row_ && row_->desc)
        return QString::fromUtf8(row_->desc);
    return QString();
}

const QString Row::class_name() const
{
    if (row_ && row_->desc)
        return QString::fromUtf8(row_->desc);
    return QString();
}

int Row::index() const
{
    return index_;
}

bool Row::operator<(const Row &other) const
{
    return (decoder_ < other.decoder_) ||
        (decoder_ == other.decoder_ && row_ < other.row_);
}
