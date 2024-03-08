#pragma once
#include <cmath>
#include <functional>
#include <vector>
#define M_PI 3.14159265358979323846

namespace nodesoup {

    // 使用邻接表的图结构
    using vertex_id_t = std::size_t;
    using adj_list_t = std::vector<std::vector<vertex_id_t>>;           // 邻接表

    // 基本类
    struct Vector2D;

    // 点的坐标
    struct Point2D {
        double x;
        double y;
        explicit operator Vector2D() const;
        // 根据向量更新点的坐标
        Point2D& operator+=(const Vector2D& vector);
        Point2D& operator-=(const Vector2D& vector);
    };

    // 重心点
    struct MassPoint : public Point2D {
        int mass;
        double size;
        MassPoint(Point2D& p, int m, double sz) {
            x = p.x;
            y = p.y;
            mass = m;
            size = sz;
        }
    };


    // 两点的距离
    struct Vector2D {
        double dx;
        double dy;

        double norm() const {
            return sqrt(dx * dx + dy * dy);
        }

        explicit operator Point2D() const;
        // 更新向量的值
        Vector2D& operator+=(const Vector2D& other);
        Vector2D& operator-=(const Vector2D& other);
        // 等比例缩放
        Vector2D& operator*=(double scalar);
        Vector2D& operator/=(double scalar);
    };

    // 迭代执行函数(每次迭代执行，用于单步输出)
    using iter_callback_t = std::function<void(const std::vector<Point2D>&, int)>;

    // 在邻接表表示的图g上使用fr算法，产生的图大小为width * height，迭代iters_count次
    std::vector<Point2D> fruchterman_reingold(
        const adj_list_t& g,
        unsigned int width,
        unsigned int height,
        unsigned int iters_count = 300,
        double k = 15.0,
        iter_callback_t iter_cb = nullptr);

    std::vector<Point2D> kamada_kawai(
        const adj_list_t& g,
        unsigned int width,
        unsigned int height,
        double k = 300.0,
        double energy_threshold = 1e-2);

    std::vector<Point2D> yifanhu(
        const adj_list_t& g,
        unsigned int width,
        unsigned int height,
        unsigned int iters_count = 300,
        double stepRatio = (0.95),
        double relativeStrength = (0.2),
        int quadTreeMaxLevel = 20,
        double barnesHutTheta = (1.2),
        double convergenceThreshold = (1.0E-4));

    std::vector<Point2D> sugiyama(
        const adj_list_t& g,
        unsigned int width,
        unsigned int height);

    // 对点的半径进行微调(接的边数越多，半径越大)
    std::vector<double> size_radiuses(const adj_list_t& g, double min_radius = 4.0, double k = 300.0);
}
