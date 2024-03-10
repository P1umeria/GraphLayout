#include "fruchterman_reingold.hpp"
#include "algebra.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace nodesoup {

    using std::vector;

    FruchtermanReingold::FruchtermanReingold(const adj_list_t& g, double k, double min_dis)
        : g_(g)
        , k_(k)
        , k_squared_(k* k)
        , temp_(10 * sqrt(g.size()))
        , mvmts_(g_.size()) {
        if (min_dis < 0.0) {
            min_dis = 1.0 / (g.size() * 0.8);
        }
        min_distance_rate = min_dis;
    }

    void FruchtermanReingold::operator()(vector<Point2D>& positions) {
        if (positions.size() == 0 || positions.size() != g_.size()) {
            return;
        }
        Vector2D zero = { 0.0, 0.0 };
        fill(mvmts_.begin(), mvmts_.end(), zero);

        // ���㷶Χ��С
        double min_x = positions[0].x;
        double max_x = positions[0].x;
        double min_y = positions[0].y;
        double max_y = positions[0].y;
        for (vertex_id_t v_id = 0; v_id < g_.size(); v_id++) {
            if (positions[v_id].x < min_x) {
                min_x = positions[v_id].x;
            }
            if (positions[v_id].y < min_y) {
                min_y = positions[v_id].y;
            }
            if (positions[v_id].x > max_x) {
                max_x = positions[v_id].x;
            }
            if (positions[v_id].y > max_y) {
                max_y = positions[v_id].y;
            }
        }
        double temp_dis = max_x - min_x;
        if (max_y - min_y < temp_dis) {
            temp_dis = max_y - min_y;
        }
        temp_dis *= min_distance_rate;

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
                // ������ڿ���������
                if (distance < temp_dis) {
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
