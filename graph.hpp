#pragma once

#include <vector>
#include <iostream>
#include <ostream>
#include <algorithm>

#include "utils.hpp"
#include "types.hpp"

// 有向图
class graph {
public:

    // 增加新节点，返回节点编号
    vertex_t add_node() {
        m_out_neighbours.emplace_back();
        m_in_neighbours.emplace_back();
        return m_out_neighbours.size() - 1;
    }

    /**
     * 增加一条新的边
     *
     * 同一条边不能add两次
     *
     * @param from 边起点
     * @param to   边终点
     *
     * @return 图指针
     */
    graph& add_edge(vertex_t from, vertex_t to) {
        m_out_neighbours[from].push_back(to);
        m_in_neighbours[to].push_back(from);
        return *this;
    }

    /**
     * 获得顶点数
     *
     * @return 顶点数
     */
    unsigned size() const { return m_out_neighbours.size(); }

    /**
     * 出度节点列表
     */
    const std::vector<vertex_t>& out_neighbours(vertex_t u) const { return m_out_neighbours[u]; }
    /**
     * 入度节点列表
     */
    const std::vector<vertex_t>& in_neighbours(vertex_t u) const { return m_in_neighbours[u]; }

    /**
     * 获取一个由实现定义的对象，可用于遍历顶点。
     *
     * 返回的对象可用于范围for循环。
     * 它提供了begin()和end()方法，用于生成用于遍历顶点的前向迭代器。
     */
    range<vertex_t> vertices() const { return range<vertex_t>(0, size(), 1); }

    /**
     * 移除给定的边
     */
    void remove_edge(vertex_t from, vertex_t to) {
        remove_neighour(m_out_neighbours[from], to);
        remove_neighour(m_in_neighbours[to], from);
    }

    friend std::ostream& operator<<(std::ostream& out, const graph& g) {
        for (auto u : g.vertices()) {
            out << u << ": [";

            const char* sep = "";
            for (auto v : g.out_neighbours(u)) {
                out << sep << v;
                sep = ", ";
            }

            out << "]\n";
        }
        return out;
    }

private:
    std::vector< std::vector<vertex_t> > m_out_neighbours;
    std::vector< std::vector<vertex_t> > m_in_neighbours;

    void remove_neighour(std::vector<vertex_t>& neighbours, vertex_t u) {
        auto it = std::find(neighbours.begin(), neighbours.end(), u);
        if (it != neighbours.end()) {
            neighbours.erase(it);
        }
    }

};


/**
 * 用于手动创建图形的构建器类。
 * 应谨慎使用，因为节点标识由调用者选择。
 * 所选标识应为连续的数字序列[0, n-1]，其中n是图中顶点的数量。
 * 否则，范围[0, max_id]中未使用的标识符将被添加到图中作为顶点，但没有任何边，这可能不是您想要的。
 * 例如，如果添加了边(0,2), (2,3), (0,3)，
 * 则生成的图形将包含具有标识符1的额外顶点。
 */
struct graph_builder {
    graph g;

    /**
     * 添加具有给定标识符的新边
     */
    graph_builder& add_edge(vertex_t u, vertex_t v) {
        add_vertex(u);
        add_vertex(v);
        g.add_edge(u, v);
        return *this;
    }

    /**
     * 获得最终的graph
     */
    graph build() { return g; }

    void add_vertex(vertex_t u) {
        while (u >= g.size()) {
            g.add_node();
        }
    }
};
