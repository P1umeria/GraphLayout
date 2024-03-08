#include "fruchterman_reingold.hpp"
#include "algebra.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace nodesoup {

    using std::vector;

    FruchtermanReingold::FruchtermanReingold(const adj_list_t& g, double k)
        : g_(g)
        , k_(k)
        , k_squared_(k* k)
        , temp_(10 * sqrt(g.size()))
        , mvmts_(g_.size()) {}

    void FruchtermanReingold::operator()(vector<Point2D>& positions) {
        Vector2D zero = { 0.0, 0.0 };
        fill(mvmts_.begin(), mvmts_.end(), zero);

        // ������֮��ĳ���
        for (vertex_id_t v_id = 0; v_id < g_.size(); v_id++) {
            for (vertex_id_t other_id = v_id + 1; other_id < g_.size(); other_id++) {
                if (v_id == other_id) {
                    continue;
                }
                // ��������ļ��
                Vector2D delta = positions[v_id] - positions[other_id];
                double distance = delta.norm();
                // TODO: �������Ϊ0�����

                // ���� > 1000.0: ����
                if (distance > 1000.0) {
                    continue;
                }
                // ������ϵ��(�����㵱ǰ����ɷ���)
                double repulsion = k_squared_ / distance;
                // ��һ��
                delta = delta / distance;
                // ����������
                mvmts_[v_id] += delta * repulsion;
                mvmts_[other_id] -= delta * repulsion;
            }

            // ��֮���������
            for (vertex_id_t adj_id : g_[v_id]) {
                if (adj_id > v_id) {
                    continue;
                }

                Vector2D delta = positions[v_id] - positions[adj_id];
                double distance = delta.norm();
                if (distance == 0.0) {
                    continue;
                }
                // ��ֱ�����ߵĵ��������ϵ��(����������)
                double attraction = distance * distance / k_;
                // ��һ��
                delta = delta / distance;
                // ����ֱ�����ߵ���������
                mvmts_[v_id] -= delta * attraction;
                mvmts_[adj_id] += delta * attraction;
            }
        }

        // �����¶�Լ����ǰ����ƶ�����
        for (vertex_id_t v_id = 0; v_id < g_.size(); v_id++) {
            double mvmt_norm = mvmts_[v_id].norm();
            // < 1.0: ����Լ������
            if (mvmt_norm < 1.0) {
                continue;
            }
            // �����ƶ��ľ���
            double capped_mvmt_norm = std::min(mvmt_norm, temp_);
            Vector2D capped_mvmt = mvmts_[v_id] / mvmt_norm * capped_mvmt_norm;
            // ���µ�ĵ�ǰλ��
            positions[v_id] += capped_mvmt;
        }

        // ģ���˻�ֱ��ϵ��ά����1.5
        if (temp_ > 1.5) {
            temp_ *= 0.85;
        }
        else {
            temp_ = 1.5;
        }
    }
}
