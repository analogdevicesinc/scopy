#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <cstdint>
#include <vector>

#include <QString>

struct srd_proto_data;
class Row;

using std::vector;

class Annotation
{
public:
    typedef uint32_t Class;

public:
    Annotation() = default;
    Annotation(const Annotation &other) = default;
    Annotation(const srd_proto_data *const pdata, const Row *row);

    uint64_t start_sample() const;
    uint64_t end_sample() const;
    Class ann_class() const;
    const vector<QString>& annotations() const;
    const Row* row() const;

    bool operator<(const Annotation &other) const;

private:
    uint64_t start_sample_;
    uint64_t end_sample_;
    Class ann_class_;
    vector<QString> annotations_;
    const Row *row_;

};

#endif // ANNOTATION_H
