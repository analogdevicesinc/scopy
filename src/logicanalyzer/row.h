#ifndef ROW_H
#define ROW_H

#include <libsigrokdecode/libsigrokdecode.h>

#include <QString>

class Row
{
public:
    Row();

    Row(int index, const srd_decoder *decoder,
        const srd_decoder_annotation_row *row = nullptr);

    const srd_decoder* decoder() const;
    const srd_decoder_annotation_row* row() const;

    const QString title() const;
    const QString class_name() const;
    int index() const;

    bool operator<(const Row &other) const;

private:
    int index_;
    const srd_decoder *decoder_;
    const srd_decoder_annotation_row *row_;
};

#endif // ROW_H
