#pragma once

#include <memory>
#include <set>
#include <assert.h>

// CREDIT: some guy on some blog thx for this overcomplicated paste

struct PoisonRange {
    const unsigned char *bp;
    size_t len;

public:
    PoisonRange (const unsigned char *bp, size_t len) :
        bp (bp),
        len (len)
    {
        assert(len > 0);
    }

    // Sort ranges by their start pointer (no overlap is guaranteed)
    bool operator<(const PoisonRange &other) const {
        return bp < other.bp;
    }
};

// Set is kept sorted by default (uses operator< on PoisonRange)
std::set<PoisonRange> poison_ranges;

// Sample XOR bytestring :-)
const unsigned char a_bad_idea[4] = {0xAB, 0xAD, 0x1D, 0xEA};

void Poison_Memory(void *p, size_t len)
{
    auto *bp = reinterpret_cast<unsigned char*>(p);
    assert(reinterpret_cast<uintptr_t>(bp) % sizeof(uintptr_t) == 0);
    assert(len % sizeof(uintptr_t) == 0);

    // Lowest range we might overlap (bp >= low->bp)
    auto it_low = poison_ranges.lower_bound(PoisonRange {bp, len});
    // http://stackoverflow.com/a/23011602/211160
    if (it_low != std::begin(poison_ranges)) {
        if (it_low->bp != bp)
            it_low--;
    }
    else {
        // No smaller element found
        it_low = std::end(poison_ranges);
    }

    // Highest range we *cannot* overlap (bp + len <= high->bp)
    auto it_high = poison_ranges.lower_bound(PoisonRange {bp + len, len});

    if (
        it_low == std::end(poison_ranges)
        && it_high == std::end(poison_ranges)
    ) {
        // No contentions
        poison_ranges.emplace(bp, len);
    }
    else if (it_high == std::end(poison_ranges)) {
        const PoisonRange low = *it_low;

        if (low.bp + low.len == bp) {
            // end of previous matches our start; merge ranges
            it_low = poison_ranges.erase(it_low);
            poison_ranges.emplace_hint(it_low, low.bp, low.len + len);
        }
        else {
            // No overlap, make a new range
            assert(low.bp + low.len < bp);
            poison_ranges.emplace_hint(it_low, bp, len);
        }
    }
    else if (it_low == std::end(poison_ranges)) {
        const PoisonRange high = *it_high;

        if (bp + len == high.bp) {
            // end of previous matches our start; merge ranges
            it_high = poison_ranges.erase(it_high);
            poison_ranges.emplace_hint(it_high, bp, len + high.len);
        }
        else {
            // No overlap, make a new range
            assert(bp + len < high.bp);
            poison_ranges.emplace_hint(it_high, bp, len);
        }
    }
    else {
        const PoisonRange low = *it_low;
        const PoisonRange high = *it_high;

        // We cannot poison a range that straddles any existing poison
        // ranges, so the range past our end must be immediately after
        // the range before our start
        assert(std::next(it_low, 1) == it_high);

        if (low.bp + low.len == bp && bp + len == high.bp) {
            // Closes a gap precisely, so we wind up net *removing* a range
            poison_ranges.erase(it_low);
            it_high = poison_ranges.erase(it_high);
            poison_ranges.emplace_hint(
                it_high, low.bp, low.len + len + high.len
            );
        }
        else {
            if (low.bp + low.len == bp) {
                // end of previous matches our start; merge with previous
                it_low = poison_ranges.erase(it_low);
                poison_ranges.emplace_hint(it_low, low.bp, low.len + len);
            }
            else if (bp + len == high.bp) {
                // start of next matches our end; merge with next
                it_high = poison_ranges.erase(it_high);
                poison_ranges.emplace_hint(it_high, bp, len + high.len);
            }
            else {
                // No merge, so just put a new segment that lives between
                poison_ranges.emplace_hint(it_high, bp, len);
            }
        }
    }

    // Since we didn't overlap regions, it should be okay to scramble the
    // memory in the poisoning range
    while (len)
        *bp++ = a_bad_idea[len-- % 4];
}

void Unpoison_Memory(void *p, size_t len)
{
    auto *bp = reinterpret_cast<unsigned char*>(p);
    assert(reinterpret_cast<uintptr_t>(bp) % sizeof(uintptr_t) == 0);
    assert(len % sizeof(uintptr_t) == 0);

    // Invariant is that all ranges are maintained in the poisoning list
    // contiguously.  Hence the unpoisoning should not be able to straddle
    // an already unpoisoned range.

    // Lowest range we might overlap (bp >= low->bp)
    auto it = poison_ranges.lower_bound(PoisonRange {bp, len});
    // http://stackoverflow.com/a/23011602/211160
    if (it != std::begin(poison_ranges)) {
        if (it->bp != bp)
            it--;
    }
    else {
        if (it->bp != bp) {
            // No smaller element found
            assert(false);
        }
    }

    assert(it != std::end(poison_ranges));

    // Because we're not using a <multiset>, you can't add entries with the
    // range's key while the range is still there, and iterator is const
    const PoisonRange range = *it;
    it = poison_ranges.erase(it);

    assert(bp >= range.bp);
    assert(bp + len <= range.bp + range.len);

    if (range.bp == bp && range.len == len) {
        // Add nothing - we're going to unpoison the whole range
    }
    else if (range.bp == bp) {
        // We're chopping a bit off the head of the range
        poison_ranges.emplace_hint(it, range.bp + len, range.len - len);
    }
    else if (bp + len == range.bp + range.len) {
        // We're chopping a bit off the tail of the range
        poison_ranges.emplace_hint(it, range.bp, range.len - len);
    }
    else {
        // in-the-middle: split the range in two with the unpoisoned range
        it = poison_ranges.emplace_hint(
            it, bp + len, (range.bp + range.len) - (bp + len)
        );
        poison_ranges.emplace_hint(it, range.bp, bp - range.bp);
    }

    // Unscramble the memory.
    while (len)
        *bp++ = a_bad_idea[len-- % 4];
}
