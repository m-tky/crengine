// Standalone regression tests; run tests/run-fork-render-helpers.sh.
#include "../crengine/include/lvfntman_vert_buffer.h"
#include "../crengine/include/lvlogical_cascade.h"
#include "../crengine/include/lvrend_table_edges.h"
#include <cassert>
#include <set>
#include <vector>

static void testRotationBuffer() {
    int width = 52;
    const int height = 4;
    unsigned char *buffer = new unsigned char[width * height]();
    for (int y = 0; y < height; ++y)
        for (int x = 0; x < width; ++x)
            buffer[y * width + x] = (unsigned char)(1 + x + y);
    ensureVerticalRotationBufferWidth(buffer, width, height, 61);
    assert(width >= 61);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < 52; ++x)
            assert(buffer[y * width + x] == 1 + x + y);
        for (int x = 52; x < width; ++x)
            assert(buffer[y * width + x] == 0);
    }
    // A second expansion must preserve pixels at the old row-stride boundary.
    int previous_width = width;
    buffer[(height - 1) * width + width - 1] = 127;
    ensureVerticalRotationBufferWidth(buffer, width, height, previous_width + 1);
    assert(width > previous_width);
    assert(buffer[(height - 1) * width + previous_width - 1] == 127);
    assert(buffer[(height - 1) * width + previous_width] == 0);
    unsigned char *unchanged = buffer;
    ensureVerticalRotationBufferWidth(buffer, width, height, previous_width + 1);
    assert(buffer == unchanged);
    delete[] buffer;
}

static void testLogicalCascade() {
    // Use the actual one-byte scratch representation; preserve upstream bits.
    signed char flags = 3;
    assert(!ForkLogicalCascade::needsReplay(flags, false));
    assert(!ForkLogicalCascade::needsReplay(flags, true));
    assert(!ForkLogicalCascade::useVertical(flags, false));
    assert(!ForkLogicalCascade::needsReplay(flags, false));
    assert(ForkLogicalCascade::needsReplay(flags, true));
    ForkLogicalCascade::clear(flags);
    assert(flags == 3);
    assert(ForkLogicalCascade::useVertical(flags, true));
    assert(!ForkLogicalCascade::needsReplay(flags, true));
    assert(ForkLogicalCascade::needsReplay(flags, false));
    // Initial and final modes may agree while a middle declaration did not.
    assert(!ForkLogicalCascade::useVertical(flags, false));
    assert(ForkLogicalCascade::needsReplay(flags, true));
    assert(ForkLogicalCascade::needsReplay(flags, false));
    for (bool vertical : {false, true}) {
        flags = 3 | ForkLogicalCascade::overrideFor(vertical);
        assert(ForkLogicalCascade::isReplay(flags));
        assert(ForkLogicalCascade::useVertical(flags, !vertical) == vertical);
        assert(!ForkLogicalCascade::needsReplay(flags, vertical));
        ForkLogicalCascade::clear(flags);
        assert(flags == 3 && !ForkLogicalCascade::isReplay(flags));
    }
}

struct Cell {
    int row, col, rows, cols;
};

static void testTableEdges() {
    // Mixed spans and unsorted insertion: compare both axes with a small
    // exhaustive oracle. This is a correctness test, not a speed benchmark.
    std::vector<Cell> cells = {
        {0, 0, 2, 1}, {0, 1, 1, 2}, {1, 1, 1, 1}, {1, 2, 2, 1},
        {2, 0, 1, 2}, {3, 0, 1, 1}, {3, 1, 1, 2},
    };
    ForkTableEdgeIndex<Cell> inline_ends, block_ends;
    for (auto it = cells.rbegin(); it != cells.rend(); ++it) {
        inline_ends.add(it->col + it->cols, it->row, it->row + it->rows, &*it);
        block_ends.add(it->row + it->rows, it->col, it->col + it->cols, &*it);
    }
    inline_ends.sort();
    block_ends.sort();
    for (auto &cell : cells) {
        std::set<Cell *> expected_inline, expected_block, actual_inline, actual_block;
        for (auto &other : cells) {
            if (other.col + other.cols == cell.col && other.row < cell.row + cell.rows
                    && other.row + other.rows > cell.row)
                expected_inline.insert(&other);
            if (other.row + other.rows == cell.row && other.col < cell.col + cell.cols
                    && other.col + other.cols > cell.col)
                expected_block.insert(&other);
        }
        inline_ends.forEachOverlapping(cell.col, cell.row, cell.row + cell.rows,
            [&](Cell *other) { actual_inline.insert(other); });
        block_ends.forEachOverlapping(cell.row, cell.col, cell.col + cell.cols,
            [&](Cell *other) { actual_block.insert(other); });
        assert(expected_inline == actual_inline);
        assert(expected_block == actual_block);
    }
    // Conflicting spans can have unsorted end coordinates.
    ForkTableEdgeIndex<Cell> overlapping;
    overlapping.add(4, 0, 4, &cells[0]);
    overlapping.add(4, 2, 3, &cells[1]);
    overlapping.sort();
    std::vector<Cell *> matches;
    overlapping.forEachOverlapping(4, 3, 4,
        [&](Cell *other) { matches.push_back(other); });
    assert(matches.size() == 1 && matches[0] == &cells[0]);
    // Equal-border winners use insertion order, not spatial order.
    ForkTableEdgeIndex<Cell> ordered;
    ordered.add(2, 1, 2, &cells[0]);
    ordered.add(2, 0, 1, &cells[1]);
    ordered.sort();
    matches.clear();
    ordered.forEachOverlapping(2, 0, 2,
        [&](Cell *other) { matches.push_back(other); });
    assert(matches.size() == 2);
    assert(matches[0] == &cells[0] && matches[1] == &cells[1]);
}

int main() {
    testRotationBuffer();
    testLogicalCascade();
    testTableEdges();
}
