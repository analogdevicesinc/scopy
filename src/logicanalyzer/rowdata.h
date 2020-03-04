#ifndef ROWDATA_H
#define ROWDATA_H

#include <libsigrokdecode/libsigrokdecode.h>

#include "annotation.h"

#include <vector>
#include <set>

class Row;

class RowData
{
public:
    RowData() = default;

public:
    uint64_t get_max_sample() const;

    uint64_t size() const {
        return annotations_.size();
    }

    /**
     * Extracts annotations between the given sample range into a vector.
     * Note: The annotations are unsorted and only annotations that fully
     * fit into the sample range are considered.
     */
    void get_annotation_subset(
        vector<Annotation> &dest,
        uint64_t start_sample, uint64_t end_sample) const;

    void emplace_annotation(srd_proto_data *pdata, const Row *row);

    std::pair<uint64_t, uint64_t> get_annotation_subset(uint64_t start_sample,
                                                        uint64_t end_sample) const;

    Annotation getAnnAt(uint64_t index) const;

    void sort_annotations();

private:
    struct annotation_compare {
        bool operator() (const Annotation &a, const Annotation &b) const {
            return a.start_sample() < b.start_sample();
        }
    };

    std::vector<Annotation/*, annotation_compare*/> annotations_;
};

#endif // ROWDATA_H
