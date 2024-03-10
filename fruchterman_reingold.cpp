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

        // 计算范围大小
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

        // 计算点对之间的斥力
        for (vertex_id_t v_id = 0; v_id < g_.size(); v_id++) {
            for (vertex_id_t other_id = v_id + 1; other_id < g_.size(); other_id++) {
                if (v_id == other_id) {
                    continue;
                }
                // 计算两点的间距
                Vector2D delta = positions[v_id] - positions[other_id];
                double distance = delta.norm();
                // TODO: 解决距离为0的情况

                // 距离 > 1000.0: 忽略
                if (distance > 1000.0) {
                    continue;
                }
                // 斥力的系数(和两点当前距离成反比)
                double repulsion = k_squared_ / distance;
                // 归一化
                delta = delta / distance;
                // 将两点拉开
                mvmts_[v_id] += delta * repulsion;
                mvmts_[other_id] -= delta * repulsion;
            }

            // 边之间的吸引力
            for (vertex_id_t adj_id : g_[v_id]) {
                if (adj_id > v_id) {
                    continue;
                }

                Vector2D delta = positions[v_id] - positions[adj_id];
                double distance = delta.norm();
                // 如果过于靠近，忽略
                if (distance < temp_dis) {
                    continue;
                }
                // 有直接连线的点的吸引力系数(与距离成正比)
                double attraction = distance * distance / k_;
                // 归一化
                delta = delta / distance;
                // 将有直接连边的两点拉近
                mvmts_[v_id] -= delta * attraction;
                mvmts_[adj_id] += delta * attraction;
            }
        }

        // 根据温度约束当前点的移动距离
        for (vertex_id_t v_id = 0; v_id < g_.size(); v_id++) {
            double mvmt_norm = mvmts_[v_id].norm();
            // < 1.0: 忽略约束条件
            if (mvmt_norm < 1.0) {
                continue;
            }
            // 更新移动的距离
            double capped_mvmt_norm = std::min(mvmt_norm, temp_);
            Vector2D capped_mvmt = mvmts_[v_id] / mvmt_norm * capped_mvmt_norm;
            // 更新点的当前位置
            positions[v_id] += capped_mvmt;
        }

        // 模拟退火，直到系数维持在1.5
        if (temp_ > 1.5) {
            temp_ *= 0.85;
        }
        else {
            temp_ = 1.5;
        }
    }
}
