#pragma once
#include <cmath>
#include <functional>
#include <vector>
#define M_PI 3.14159265358979323846

namespace nodesoup {

    // ʹ���ڽӱ��ͼ�ṹ
    using vertex_id_t = std::size_t;
    using adj_list_t = std::vector<std::vector<vertex_id_t>>;           // �ڽӱ�

    // ������
    struct Vector2D;

    // �������
    struct Point2D {
        double x;
        double y;
        explicit operator Vector2D() const;
        // �����������µ������
        Point2D& operator+=(const Vector2D& vector);
        Point2D& operator-=(const Vector2D& vector);
    };

    // ���ĵ�
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


    // ����ľ���
    struct Vector2D {
        double dx;
        double dy;

        double norm() const {
            return sqrt(dx * dx + dy * dy);
        }

        explicit operator Point2D() const;
        // ����������ֵ
        Vector2D& operator+=(const Vector2D& other);
        Vector2D& operator-=(const Vector2D& other);
        // �ȱ�������
        Vector2D& operator*=(double scalar);
        Vector2D& operator/=(double scalar);
    };

    // ����ִ�к���(ÿ�ε���ִ�У����ڵ������)
    using iter_callback_t = std::function<void(const std::vector<Point2D>&, int)>;

    // ���ڽӱ��ʾ��ͼg��ʹ��fr�㷨��������ͼ��СΪwidth * height������iters_count��
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

    // �Ե�İ뾶����΢��(�ӵı���Խ�࣬�뾶Խ��)
    std::vector<double> size_radiuses(const adj_list_t& g, double min_radius = 4.0, double k = 300.0);
}
