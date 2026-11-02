#ifndef LVREND_TABLE_EDGES_H_INCLUDED
#define LVREND_TABLE_EDGES_H_INCLUDED

#include <algorithm>
#include <map>
#include <vector>

// Index cell edges by their grid boundary. Prefix maxima also support
// overlapping spans in malformed tables. Valid grids visit only adjacent
// edges, without scanning every cell pair.
template <typename Cell> class ForkTableEdgeIndex {
    struct Edge {
        int start;
        int end;
        int prefix_end;
        std::size_t order;
        Cell *cell;
    };
    std::map<int, std::vector<Edge> > boundaries;
    std::size_t next_order = 0;
public:
    void add(int boundary, int start, int end, Cell *cell) {
        boundaries[boundary].push_back({start, end, end, next_order++, cell});
    }
    void sort() {
        for (auto &entry : boundaries) {
            std::sort(entry.second.begin(), entry.second.end(),
                [](const Edge &a, const Edge &b) { return a.start < b.start; });
            int prefix_end = 0;
            for (auto &edge : entry.second) {
                prefix_end = std::max(prefix_end, edge.end);
                edge.prefix_end = prefix_end;
            }
        }
    }
    template <typename Visit> void forEachOverlapping(int boundary, int start, int end, Visit visit) const {
        auto found = boundaries.find(boundary);
        if (found == boundaries.end())
            return;
        const auto &edges = found->second;
        auto edge = std::lower_bound(edges.begin(), edges.end(), start,
            [](const Edge &edge, int start) { return edge.prefix_end <= start; });
        const Edge *single = NULL;
        std::vector<const Edge *> matches;
        for (; edge != edges.end() && edge->start < end; ++edge) {
            if (edge->end <= start)
                continue;
            if (!single) {
                single = &*edge;
            } else {
                if (matches.empty())
                    matches.push_back(single);
                matches.push_back(&*edge);
            }
        }
        // Ordinary grid queries have zero or one neighbour: no heap allocation
        // or sorting is needed. Spanning edges still preserve border tie order.
        if (matches.empty()) {
            if (single)
                visit(single->cell);
            return;
        }
        // Border ties must retain the original row-major traversal order,
        // which may differ from spatial order for spanning cells.
        std::sort(matches.begin(), matches.end(),
            [](const Edge *a, const Edge *b) { return a->order < b->order; });
        for (const Edge *match : matches)
            visit(match->cell);
    }
};

#endif // LVREND_TABLE_EDGES_H_INCLUDED
