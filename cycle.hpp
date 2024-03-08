#pragma once

#include <vector>
#include <limits>

#include "subgraph.hpp"

namespace detail {


    /**
     * 存储了被反转以消除循环的边。
     * 这个结构体包含了以它们反转形式存储的边
     * 即如果边 (u, v) 导致了一个循环，那么它会保存为 (v, u)
     */
    struct feedback_set {
        edge_set reversed;
        edge_set removed;
        std::vector<vertex_t> loops;
    };


    /**
     * 环消除算法接口
     */
    struct cycle_removal {

        /**
         * 修改输入图，通过反转某些边来消除循环
         *
         * @return 翻转的边
         */
        virtual feedback_set run(subgraph& g) = 0;

        virtual ~cycle_removal() = default;
    };


    /**
     * 使用深度优先搜索来移除图中的环的算法
     */
    class dfs_removal : public cycle_removal {

        enum class state : char { done, in_progress, unvisited };
        vertex_map<state> marks;

    public:
        feedback_set run(subgraph& g) override {
            marks.init(g, state::unvisited);

            // 寻找导致环的边
            std::vector<edge> to_reverse;
            std::vector<edge> to_remove;
            for (auto u : g.vertices()) {
                if (marks[u] == state::unvisited) {
                    dfs(g, u, to_reverse, to_remove);
                }
            }

            // 删除或翻转边
            feedback_set reversed_edges;

            for (auto e : to_remove) {
                g.remove_edge(e);
                if (e.from == e.to)
                    reversed_edges.loops.push_back(e.from);
                else
                    reversed_edges.removed.insert(reversed(e));
            }

            for (auto e : to_reverse) {
                g.remove_edge(e);
                g.add_edge(reversed(e));
                reversed_edges.reversed.insert(reversed(e));
            }

            return reversed_edges;
        }

    private:

        void dfs(subgraph& g, vertex_t u, std::vector<edge>& to_reverse, std::vector<edge>& to_remove) {
            marks[u] = state::in_progress;

            for (auto v : g.out_neighbours(u)) {
                if (u == v) { // 自环
                    to_remove.push_back({ u, u });
                }
                else if (marks[v] == state::in_progress) { // 有环
                    if (g.has_edge(v, u)) { // 双向边环
                        to_remove.push_back({ u, v });
                    }
                    else { // 常规环
                        to_reverse.push_back({ u, v });
                    }
                }
                else if (marks[v] == state::unvisited) {
                    dfs(g, v, to_reverse, to_remove);
                }
            }

            marks[u] = state::done;
        }
    };

} //namespace detail
