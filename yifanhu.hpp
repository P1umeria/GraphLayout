#pragma once
#include "nodesoup.hpp"
#include <utility>
#include <vector>
#include <queue>

namespace nodesoup {
    class quadtree
    {
    private:
        // static int node_num;                // �ڵ���Ŀ
        static bool description;            // �Ƿ��������
        Point2D origin;                     // ������С��x, y����
        double size;                        // �����С
        bool isleaf;                        // �Ƿ�ΪҶ�ӽڵ�
        int max_dep;                        // ��������ķ����
        int mass;                           // ��������
        quadtree** children;    // ��������
        Point2D mass_center;                // ��������
    public:
        quadtree(Point2D p, double sz, int max_dep);
        void quad_divide();                 // �������ĵȷ֣���������
        void assimilateNode(Point2D p);     // ������������
        bool quad_insert(Point2D p);        // �ڵ��������
        bool insert_node(Point2D p);        // �����½ڵ�
        void get_farthest_mass(Point2D p, double threshold, std::vector<MassPoint>& pos_arr);
        // ��ȡ��õ�(����/����)��С��������ֵ�����е������
        void get_all_mass_center(std::vector<MassPoint>& center);
        // ��ȡ���в�Ϊ�յ����������
        void cal_electrical_forces(Point2D p, double threshold, double c, double k, Point2D& e_force);
        // e_forceΪp�������Ŀ�����threshold���ڵ����е����ʩ����ƫ����

        ~quadtree();
    };

    class YifanHu
    {
    private:
        const adj_list_t& g_;               // ����ͼ
        int node_num;                       // �ڵ�����
        bool converged;                     // �Ƿ���ɵ�������convergenceThreshold���
        double AverageEdgeLength;           // ͼ��ƽ���߳�
        double relativeStrength;            // electric���������spring�����ı�����С������ʽ�е�C default 0.2
        double stepRatio;                   // ���������ʵĵ��� default 0.95
        int quadTreeMaxLevel;               // �Ĳ���������ķִ��� default 20
        double barnesHutTheta;              // BH�㷨��Ѱ�����ֵ(size / d < barnesHutTheta) default 1.2
        double convergenceThreshold;        // ���������ı�����ֵ default 1e-4
        bool adaptiveCooling;               // ����Ӧ�˻� default True
        double optimalDistance;             // ����Ŀ����� default relativeStrength ** (1.0 / 3) * getAverageEdgeLength()
        double step;                        // ������С
        double initialStep;                 // ��ʼ���� default optimalDistance / 5.0
        int progress;                       // ������ϵ�������ڿ��Ʋ�����С�ĸ��� default 0 and progress < 5
        double energy0;                     // ��ʼ����
        double energy;                      // ��ǰ����
    public:
        YifanHu(const adj_list_t& g);
        double getAverageEdgeLength(std::vector<Point2D>& positions);
        void resetPropertiesValues(double stepRatio = 0.95, double relativeStrength = 0.2, int quadTreeMaxLevel = 20, double barnesHutTheta = 1.2, double convergenceThreshold = 1e-4);
        void updateStep();
        bool judge_converged();
        void operator()(std::vector<Point2D>& positions);
    };

}