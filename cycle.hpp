#pragma once

#include <vector>
#include <limits>

#include "subgraph.hpp"

namespace detail {


    /**
     * �洢�˱���ת������ѭ���ıߡ�
     * ����ṹ������������Ƿ�ת��ʽ�洢�ı�
     * ������� (u, v) ������һ��ѭ������ô���ᱣ��Ϊ (v, u)
     */
    struct feedback_set {
        edge_set reversed;
        edge_set removed;
        std::vector<vertex_t> loops;
    };


    /**
     * �������㷨�ӿ�
     */
    struct cycle_removal {

        /**
         * �޸�����ͼ��ͨ����תĳЩ��������ѭ��
         *
         * @return ��ת�ı�
         */
        virtual feedback_set run(subgraph& g) = 0;

        virtual ~cycle_removal() = default;
    };


    /**
     * ʹ����������������Ƴ�ͼ�еĻ����㷨
     */
    class dfs_removal : public cycle_removal {

        enum class state : char { done, in_progress, unvisited };
        vertex_map<state> marks;

    public:
        feedback_set run(subgraph& g) override {
            marks.init(g, state::unvisited);

            // Ѱ�ҵ��»��ı�
            std::vector<edge> to_reverse;
            std::vector<edge> to_remove;
            for (auto u : g.vertices()) {
                if (marks[u] == state::unvisited) {
                    dfs(g, u, to_reverse, to_remove);
                }
            }

            // ɾ����ת��
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
                if (u == v) { // �Ի�
                    to_remove.push_back({ u, u });
                }
                else if (marks[v] == state::in_progress) { // �л�
                    if (g.has_edge(v, u)) { // ˫��߻�
                        to_remove.push_back({ u, v });
                    }
                    else { // ���滷
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
