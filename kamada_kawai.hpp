#pragma once
#include "nodesoup.hpp"
#include <vector>

namespace nodesoup {
    class KamadaKawai {
    public:
        struct Spring {
            double length;
            double strength;
        };

        KamadaKawai(const adj_list_t& g, double k = 300.0, double energy_threshold = 1e-2);
        void operator()(std::vector<Point2D>& positions) const;

    private:
        const adj_list_t& g_;
        const double energy_threshold_;
        std::vector<std::vector<Spring>> springs_;

        static std::vector<std::vector<vertex_id_t>> floyd_warshall_(const adj_list_t& g);
        // p m
        double find_max_vertex_energy_(const std::vector<Point2D>& positions, vertex_id_t& max_energy_v_id) const;
        // delta m
        double compute_vertex_energy_(vertex_id_t v_id, const std::vector<Point2D>& positions) const;
        Point2D compute_next_vertex_position_(vertex_id_t v_id, const std::vector<Point2D>& positions) const;
    };
}
