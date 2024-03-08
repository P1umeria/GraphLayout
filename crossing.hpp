#pragma once

#include <algorithm>
#include <iostream>
#include <random>
#include <cassert>

#include "layering.hpp"
#include "utils.hpp"
#include "report.hpp"

namespace detail {

    // ----------------------------------------------------------------------------------------------
    // -------------------------------  COUNTING CROSSINGS  -----------------------------------------
    // ----------------------------------------------------------------------------------------------


    // 计算点u和点v之间边相交的数目，u在v左边，计算入边和出边的和，u和v在同一层
    int crossing_number(const hierarchy& h, vertex_t u, vertex_t v) {
        int count = 0;
        for (auto out_u : h.g.out_neighbours(u)) {
            for (auto out_v : h.g.out_neighbours(v)) {
                if (h.pos[out_v] < h.pos[out_u]) {
                    ++count;
                }
            }
        }
        for (auto in_u : h.g.in_neighbours(u)) {
            for (auto in_v : h.g.in_neighbours(v)) {
                if (h.pos[in_v] < h.pos[in_u]) {
                    ++count;
                }
            }
        }
        return count;
    }

    // 统计层索引为'layer'和'layer - 1'之间的交叉数量。
    int count_layer_crossings(const hierarchy& h, int layer) {
        const std::vector<vertex_t>& upper = h.layers[layer - 1];
        int count = 0;

        for (int i = 0; i < upper.size() - 1; ++i) {
            for (auto u : h.g.out_neighbours(upper[i])) {
                int j = h.pos[u];
                for (int s = i + 1; s < upper.size(); ++s) {
                    for (auto v : h.g.out_neighbours(upper[s])) {
                        int t = h.pos[v];
                        if ((s > i && t < j) || (s < i && t > j)) {
                            ++count;
                        }
                    }
                }
            }
        }

        return count;
    }


    // 计算层次结构中的总交叉数量
    int count_crossings(const hierarchy& h) {
        int count = 0;
        for (int i = 1; i < h.size(); ++i) {
            count += count_layer_crossings(h, i);
        }
        return count;
    }


    // ----------------------------------------------------------------------------------------------
    // -------------------------------  CROSSING REDUCTION  -----------------------------------------
    // ----------------------------------------------------------------------------------------------


    /**
     * 用于交叉减少算法的接口。
     * 它重新排列层次结构的每一层以避免交叉。
     */
    struct crossing_reduction {


        /**
         * 执行算法。
         * 它应该使 h 处于一致的状态 - 排名、层和位置都与彼此一致。
         */
        virtual void run(hierarchy& h) = 0;
        virtual ~crossing_reduction() = default;
    };



    /**
     * 基于重心的启发式算法用于交叉减少。
     * 每一层的顶点根据它们的重心排序 - 即它们入度和出度点的平均位置。
     */
    class barycentric_heuristic : public crossing_reduction {
        unsigned random_iters = 1;
        unsigned forgiveness = 7;
        bool trans = true;

        std::mt19937 mt;

        vertex_map<int> best_order;
        int min_cross;

    public:
        barycentric_heuristic() = default;
        barycentric_heuristic(int rnd_iters, int max_fails, bool do_transpose)
            : random_iters(rnd_iters), forgiveness(max_fails), trans(do_transpose) {}

        void run(hierarchy& h) override {
#ifdef REPORTING
            report::base = min_cross;
            report::random_runs = max_iters;
            report::forgivness = forgiveness;
            report::transpose = trans;
            report::random_final.clear();
            report::iters = 0;
#endif

            min_cross = init_order(h);
            best_order = h.pos;
            int base = min_cross;
            for (int i = 0; i < random_iters; ++i) {
                reduce(h, base);

                if (i != random_iters - 1) {
                    for (auto& l : h.layers) {
                        std::shuffle(l.begin(), l.end(), mt);
                    }
                    h.update_pos();
                    base = init_order(h);
                }
            }

            for (auto u : h.g.vertices()) {
                h.layer(u)[best_order[u]] = u;
            }
            h.update_pos();

#ifdef REPORTING
            report::final = min_cross;
#endif
        }

        int init_order(hierarchy& h) {
            barycenter(h, 0);
            return count_crossings(h);
        }

    private:
        // 尝试减少交叉数量
        void reduce(hierarchy& h, int local_min) {
            auto local_order = h.pos;
            int fails = 0;

            for (int i = 0; ; ++i) {

                barycenter(h, i);

                if (trans) {
#ifdef FAST
                    fast_transpose(h);
#else
                    transpose(h);
#endif
                }

                int cross = count_crossings(h);
                //std::cout << cross << "\n";
                if (cross < local_min) {
                    fails = 0;
                    local_order = h.pos;
                    local_min = cross;
                }
                else {
                    fails++;
                }

                if (fails >= forgiveness) {
                    break;
                }
            }

            if (local_min < min_cross) {
                best_order = local_order;
                min_cross = local_min;
            }

#ifdef REPORTING
            report::iters += i;
            report::random_final.push_back(local_min);
#endif
        }

        void barycenter(hierarchy& h, int i) {
            vertex_map<float> weights(h.g);

            if (i % 2 == 0) { // 自顶向下
                for (int j = 1; j < h.size(); ++j) {
                    reorder_layer(h, weights, j, true);
                }
            }
            else { // 自底向上
                for (int j = h.size() - 2; j >= 0; --j) {
                    reorder_layer(h, weights, j, false);
                }
            }
        }

        // 根据它们的权重重新排列层级“i”上的顶点
        void reorder_layer(hierarchy& h, vertex_map<float>& weights, int i, bool downward) {
            auto& layer = h.layers[i];
            for (vertex_t u : layer) {
                weights[u] = weight(h.pos, u, downward ? h.g.in_neighbours(u) : h.g.out_neighbours(u));
            }
            std::sort(layer.begin(), layer.end(), [&weights](const auto& u, const auto& v) {
                return weights[u] < weights[v];
                });

            h.update_pos();
        }

        // 计算顶点的权重，作为其相邻顶点位置的平均值
        template<typename T>
        float weight(const vertex_map<int>& positions, vertex_t u, const T& neighbours) {
            unsigned count = 0;
            unsigned sum = 0;
            for (auto v : neighbours) {
                sum += positions[v];
                count++;
            }
            if (count == 0) {
                return positions[u];
            }
            return sum / (float)count;
        }

        // 试图减少交叉的启发式方法是反复尝试交换所有相邻顶点
        void transpose(hierarchy& h) {
            bool improved = true;
            int k = 0;
            while (improved) {
                improved = false;

                for (auto& layer : h.layers) {
                    for (int i = 0; i < layer.size() - 1; ++i) {
                        int old = crossing_number(h, layer[i], layer[i + 1]);
                        int next = crossing_number(h, layer[i + 1], layer[i]);

                        if (old > next) {
                            improved = true;
                            h.swap(layer[i], layer[i + 1]);
                        }
                    }
                }
                k++;
            }
        }

        void fast_transpose(hierarchy& h) {
            //std::cout << h << "\n";
            vertex_map<bool> eligible(h.g, true);

            bool improved = true;
            int iters = 0;
            while (improved) {
                iters++;
                improved = false;
                for (auto& layer : h.layers) {
                    assert(layer.size() >= 1);
                    for (int i = 0; i < layer.size() - 1; ++i) {
                        if (eligible.at(layer[i])) {
                            int old = crossing_number(h, layer[i], layer[i + 1]);
                            int next = crossing_number(h, layer[i + 1], layer[i]);
                            int diff = old - next;

                            if (diff > 0) {
                                improved = true;
                                h.swap(layer[i], layer[i + 1]);

                                if (i > 0) eligible.set(layer[i - 1], true);
                                eligible.set(layer[i + 1], true);

                                for (auto u : h.g.neighbours(layer[i])) {
                                    eligible.set(u, true);
                                }
                                for (auto u : h.g.neighbours(layer[i + 1])) {
                                    eligible.set(u, true);
                                }
                            }
                            eligible.set(layer[i], false);
                        }
                    }
                }
            }
        }

    };

} // namespace detail