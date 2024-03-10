#include "nodesoup.hpp"
#include "fruchterman_reingold.hpp"
#include "kamada_kawai.hpp"
#include "yifanhu.hpp"
#include "sugiyama.hpp"
#include "layout.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace nodesoup {

    using std::vector;

    vector<Point2D> fruchterman_reingold(
        const adj_list_t& g,
        unsigned int width,
        unsigned int height,
        unsigned int iters_count,
        double k,
        iter_callback_t iter_cb) {
        vector<Point2D> positions(g.size());
        // 初始化到一个圆上面
        circle(g, positions);
        // 初始化
        FruchtermanReingold fr(g, k);
        for (unsigned int i = 0; i < iters_count; i++) {
            // 单步迭代
            fr(positions);
            // 中心化，迭代
            if (iter_cb != nullptr) {
                vector<Point2D> scaled_positions = positions;
                center_and_scale(g, width, height, scaled_positions);
                iter_cb(scaled_positions, i);
            }
        }

        center_and_scale(g, width, height, positions);
        //for (int v_id = 0; v_id < positions.size(); v_id++) {
        //    printf("%f %f\n", positions[v_id].x, positions[v_id].y);
        //}
        return positions;
    }

    vector<Point2D> yifanhu(
        const adj_list_t& g,
        unsigned int width,
        unsigned int height,
        unsigned int iters_count,
        double stepRatio,
        double relativeStrength,
        int quadTreeMaxLevel,
        double barnesHutTheta,
        double convergenceThreshold) {
        vector<Point2D> positions(g.size());
        // 初始化到一个圆上面
        circle(g, positions);
        // 初始化
        YifanHu yfh(g);
        yfh.getAverageEdgeLength(positions);
        yfh.resetPropertiesValues(stepRatio, relativeStrength, quadTreeMaxLevel, barnesHutTheta, convergenceThreshold);
        for (int i = 0; i < iters_count; ++i) {
            yfh(positions);
            // for(int i = 0; i < positions.size(); ++i){
            //     std::cout << positions[i].x << " " << positions[i].y << std::endl;
            // }
            if (yfh.judge_converged()) {
                break;
            }
        }

        center_and_scale(g, width, height, positions);

        return positions;
    }

    vector<Point2D> kamada_kawai(
        const adj_list_t& g,
        unsigned int width,
        unsigned int height,
        double k,
        double energy_threshold) {
        vector<Point2D> positions(g.size());
        // 初始化到一个圆上面
        circle(g, positions);
        KamadaKawai kk(g, k, energy_threshold);
        kk(positions);
        center_and_scale(g, width, height, positions);
        return positions;
    }

    std::vector<Point2D> sugiyama(
        const adj_list_t& g,
        unsigned int width,
        unsigned int height) {
        graph_builder builder;
        for (int i = 0; i < g.size(); ++i) {
            builder.add_vertex(i);
        }

        for (int i = 0; i < g.size(); ++i) {
            for (int j = 0; j < g[i].size(); ++j) {
                builder.add_edge(i, g[i][j]);
            }
        }
        graph g_ = builder.build();
        attributes attr;
        sugiyama_layout l(g_, attr);
        std::vector<Point2D> positions(g.size(), { 0, 0 });
        for (const auto& node : l.vertices()) {
            positions[node.u] = { node.pos.x, node.pos.y };
        }
        center_and_scale(g, width, height, positions);
        return positions;
    }

    vector<double> size_radiuses(const adj_list_t& g, double min_radius, double k) {
        vector<double> radiuses;
        radiuses.reserve(g.size());
        for (vertex_id_t v_id = 0; v_id < g.size(); v_id++) {
            double delta = log2(k * (double)g[v_id].size() / g.size());
            double radius = min_radius + std::max(0.0, delta);
            radiuses.push_back(radius);
        }
        return radiuses;
    }
}
