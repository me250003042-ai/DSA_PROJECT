
#ifndef ORACLE_H
#define ORACLE_H

#include <vector>
#include <unordered_map>
#include <optional>
#include <cstdint>
#include <cstddef>
#include <utility>

using ReadingID = std::uint64_t;
using ReadingValue = std::int64_t;

class Oracle {
private:
    // Sorted readings: {value, ID}
    std::vector<std::pair<ReadingValue, ReadingID>> readings;

    // Tracks active reading IDs
    std::unordered_map<ReadingID, ReadingValue> active;

public:
    bool insert(ReadingID id, ReadingValue value);

    bool erase(ReadingID id);

    std::optional<double> getMedian() const;

    std::optional<ReadingValue> getPercentile(double p) const;

    bool contains(ReadingID id) const;

    std::size_t size() const;

    bool empty() const;

    const std::vector<std::pair<ReadingValue, ReadingID>>&
    getSortedReadings() const;
};

#endif
