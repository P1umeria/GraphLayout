#pragma once

#include <vector>
#include <iostream>
#include <ostream>
#include <algorithm>

#include "utils.hpp"
#include "types.hpp"

// ����ͼ
class graph {
public:

    // �����½ڵ㣬���ؽڵ���
    vertex_t add_node() {
        m_out_neighbours.emplace_back();
        m_in_neighbours.emplace_back();
        return m_out_neighbours.size() - 1;
    }

    /**
     * ����һ���µı�
     *
     * ͬһ���߲���add����
     *
     * @param from �����
     * @param to   ���յ�
     *
     * @return ͼָ��
     */
    graph& add_edge(vertex_t from, vertex_t to) {
        m_out_neighbours[from].push_back(to);
        m_in_neighbours[to].push_back(from);
        return *this;
    }

    /**
     * ��ö�����
     *
     * @return ������
     */
    unsigned size() const { return m_out_neighbours.size(); }

    /**
     * ���Ƚڵ��б�
     */
    const std::vector<vertex_t>& out_neighbours(vertex_t u) const { return m_out_neighbours[u]; }
    /**
     * ��Ƚڵ��б�
     */
    const std::vector<vertex_t>& in_neighbours(vertex_t u) const { return m_in_neighbours[u]; }

    /**
     * ��ȡһ����ʵ�ֶ���Ķ��󣬿����ڱ������㡣
     *
     * ���صĶ�������ڷ�Χforѭ����
     * ���ṩ��begin()��end()�����������������ڱ��������ǰ���������
     */
    range<vertex_t> vertices() const { return range<vertex_t>(0, size(), 1); }

    /**
     * �Ƴ������ı�
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
 * �����ֶ�����ͼ�εĹ������ࡣ
 * Ӧ����ʹ�ã���Ϊ�ڵ��ʶ�ɵ�����ѡ��
 * ��ѡ��ʶӦΪ��������������[0, n-1]������n��ͼ�ж����������
 * ���򣬷�Χ[0, max_id]��δʹ�õı�ʶ��������ӵ�ͼ����Ϊ���㣬��û���καߣ�����ܲ�������Ҫ�ġ�
 * ���磬�������˱�(0,2), (2,3), (0,3)��
 * �����ɵ�ͼ�ν��������б�ʶ��1�Ķ��ⶥ�㡣
 */
struct graph_builder {
    graph g;

    /**
     * ��Ӿ��и�����ʶ�����±�
     */
    graph_builder& add_edge(vertex_t u, vertex_t v) {
        add_vertex(u);
        add_vertex(v);
        g.add_edge(u, v);
        return *this;
    }

    /**
     * ������յ�graph
     */
    graph build() { return g; }

    void add_vertex(vertex_t u) {
        while (u >= g.size()) {
            g.add_node();
        }
    }
};
