#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>

#include "algebra.hpp"
#include "kamada_kawai.hpp"

namespace nodesoup {

    using std::vector;


    KamadaKawai::KamadaKawai(const adj_list_t& g, double k, double energy_threshold)
        : g_(g)
        , energy_threshold_(energy_threshold) {
        vector<vector<vertex_id_t>> distances = floyd_warshall_(g_);

        // 寻找最长的路线
        size_t biggest_distance = 0;
        for (vertex_id_t v_id = 0; v_id < g_.size(); v_id++) {
            for (vertex_id_t other_id = 0; other_id < g_.size(); other_id++) {
                if (distances[v_id][other_id] != g_.size() + 1 && distances[v_id][other_id] > biggest_distance) {
                    biggest_distance = distances[v_id][other_id];
                }
            }
        }
        biggest_distance = biggest_distance + 1;

        // 理想的边长(不重要)
        double length = 1.0;
        if (biggest_distance != 0) {
            length = 1.0 / biggest_distance;
        }

        // 初始化弹簧长度和强度的矩阵(任意两个节点之间都有一个弹簧)
        for (vertex_id_t v_id = 0; v_id < g_.size(); v_id++) {
            vector<Spring> v_springs;

            for (vertex_id_t other_id = 0; other_id < g_.size(); other_id++) {
                Spring spring;
                if (v_id == other_id) {
                    spring.length = 0.0;
                    spring.strength = 0.0;
                }
                else {
                    size_t distance = distances[v_id][other_id];
                    if (distance > biggest_distance) {
                        distance = biggest_distance;
                    }
                    spring.length = distance * length;
                    spring.strength = k / (distance * distance);
                }

                v_springs.push_back(spring);
            }
            springs_.push_back(v_springs);
        }
    }

    vector<vector<vertex_id_t>> KamadaKawai::floyd_warshall_(const adj_list_t& g) {
        // 创建邻接矩阵 (infinity = 无边, 1 = 有边)
        unsigned int infinity = g.size() + 1;
        vector<vector<vertex_id_t>> distances(g.size(), vector<vertex_id_t>(g.size(), infinity));

        for (vertex_id_t v_id = 0; v_id < g.size(); v_id++) {
            distances[v_id][v_id] = 0;
            for (vertex_id_t adj_id : g[v_id]) {
                if (adj_id > v_id) {
                    distances[v_id][adj_id] = 1;
                    distances[adj_id][v_id] = 1;
                }
            }
        }

        // floyd warshall 算法, 寻找点对之间的最短路线
        for (vertex_id_t k = 0; k < g.size(); k++) {
            for (vertex_id_t i = 0; i < g.size(); i++) {
                for (vertex_id_t j = 0; j < g.size(); j++) {
                    distances[i][j] = std::min(distances[i][j], distances[i][k] + distances[k][j]);
                }
            }
        }

        return distances;
    }

#define MAX_VERTEX_ITERS_COUNT 50
#define MAX_STEADY_ENERGY_ITERS_COUNT 50

    // 减小能量，直到所有点的能量都到达阈值以下
    void KamadaKawai::operator()(vector<Point2D>& positions) const {
        vertex_id_t v_id;
        unsigned int steady_energy_count = 0;
        double max_vertex_energy = find_max_vertex_energy_(positions, v_id);

        while (max_vertex_energy > energy_threshold_ && steady_energy_count < MAX_STEADY_ENERGY_ITERS_COUNT) {
            // 逐步移动顶点，直到其能量低于阈值(牛顿法)
            unsigned int vertex_count = 0;
            do {
                positions[v_id] = compute_next_vertex_position_(v_id, positions);
                vertex_count++;
            } while (compute_vertex_energy_(v_id, positions) > energy_threshold_ && vertex_count < MAX_VERTEX_ITERS_COUNT);

            double max_vertex_energy_prev = max_vertex_energy;
            max_vertex_energy = find_max_vertex_energy_(positions, v_id);
            if (std::abs(max_vertex_energy - max_vertex_energy_prev) < 1e-20) {
                steady_energy_count++;
            }
            else {
                steady_energy_count = 0;
            }
        }
    }

    // 找到并返回最大的能量值的点的序号，返回能量值
    double KamadaKawai::find_max_vertex_energy_(const vector<Point2D>& positions, vertex_id_t& max_energy_v_id) const {
        double max_energy = -1.0;
        for (vertex_id_t v_id = 0; v_id < g_.size(); v_id++) {
            double energy = compute_vertex_energy_(v_id, positions);
            if (energy > max_energy) {
                max_energy_v_id = v_id;
                max_energy = energy;
            }
        }
        assert(max_energy != -1.0);
        return max_energy;
    }

    // 寻找 v_id 顶点和其他所有点间弹簧的能量和
    double KamadaKawai::compute_vertex_energy_(vertex_id_t v_id, const vector<Point2D>& positions) const {
        double x_energy = 0.0;
        double y_energy = 0.0;

        for (vertex_id_t other_id = 0; other_id < g_.size(); other_id++) {
            if (v_id == other_id) {
                continue;
            }

            Vector2D delta = positions[v_id] - positions[other_id];
            double distance = delta.norm();

            // delta * k * (1 - l / distance)
            Spring spring = springs_[v_id][other_id];
            x_energy += delta.dx * spring.strength * (1.0 - spring.length / distance);
            y_energy += delta.dy * spring.strength * (1.0 - spring.length / distance);
        }

        return sqrt(x_energy * x_energy + y_energy * y_energy);
    }

    // 返回能让 v_id 顶点的能量降低的位置
    Point2D KamadaKawai::compute_next_vertex_position_(vertex_id_t v_id, const vector<Point2D>& positions) const {
        double xx_energy = 0.0, xy_energy = 0.0, yx_energy = 0.0, yy_energy = 0.0;
        double x_energy = 0.0, y_energy = 0.0;

        for (vertex_id_t other_id = 0; other_id < g_.size(); other_id++) {
            if (v_id == other_id) {
                continue;
            }

            Vector2D delta = positions[v_id] - positions[other_id];
            double distance = delta.norm();
            double cubed_distance = distance * distance * distance;

            Spring spring = springs_[v_id][other_id];

            x_energy += delta.dx * spring.strength * (1.0 - spring.length / distance);
            y_energy += delta.dy * spring.strength * (1.0 - spring.length / distance);
            xy_energy += spring.strength * spring.length * delta.dx * delta.dy / cubed_distance;
            xx_energy += spring.strength * (1.0 - spring.length * delta.dy * delta.dy / cubed_distance);
            yy_energy += spring.strength * (1.0 - spring.length * delta.dx * delta.dx / cubed_distance);
        }
        yx_energy = xy_energy;

        Point2D position = positions[v_id];
        double denom = xx_energy * yy_energy - xy_energy * yx_energy;
        position.x += (xy_energy * y_energy - yy_energy * x_energy) / denom;
        position.y += (xy_energy * x_energy - xx_energy * y_energy) / denom;

        return position;
    }
}
