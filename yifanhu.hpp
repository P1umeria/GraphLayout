#pragma once
#include "nodesoup.hpp"
#include <utility>
#include <vector>
#include <queue>

namespace nodesoup {
    class quadtree
    {
    private:
        // static int node_num;                // 节点数目
        static bool description;            // 是否输出过程
        Point2D origin;                     // 区域最小的x, y坐标
        double size;                        // 区域大小
        bool isleaf;                        // 是否为叶子节点
        int max_dep;                        // 该树最大四分深度
        int mass;                           // 区域重量
        quadtree** children;    // 保存子树
        Point2D mass_center;                // 区域重心
    public:
        quadtree(Point2D p, double sz, int max_dep);
        void quad_divide();                 // 将区域四等分，生成子树
        void assimilateNode(Point2D p);     // 更新区域重心
        bool quad_insert(Point2D p);        // 节点插入子树
        bool insert_node(Point2D p);        // 加入新节点
        void get_farthest_mass(Point2D p, double threshold, std::vector<MassPoint>& pos_arr);
        // 获取与该点(距离/区域)大小不超过阈值的所有点或重心
        void get_all_mass_center(std::vector<MassPoint>& center);
        // 获取所有不为空的区域的重心
        void cal_electrical_forces(Point2D p, double threshold, double c, double k, Point2D& e_force);
        // e_force为p距离重心控制在threshold以内的所有点对其施力的偏移量

        ~quadtree();
    };

    class YifanHu
    {
    private:
        const adj_list_t& g_;               // 网络图
        int node_num;                       // 节点数量
        bool converged;                     // 是否完成迭代，跟convergenceThreshold相关
        double AverageEdgeLength;           // 图的平均边长
        double relativeStrength;            // electric斥力相对于spring引力的比例大小，即公式中的C default 0.2
        double stepRatio;                   // 步长更新率的倒数 default 0.95
        int quadTreeMaxLevel;               // 四叉树的最大四分次数 default 20
        double barnesHutTheta;              // BH算法搜寻点的阈值(size / d < barnesHutTheta) default 1.2
        double convergenceThreshold;        // 能量收敛的比例阈值 default 1e-4
        bool adaptiveCooling;               // 自适应退火 default True
        double optimalDistance;             // 最优目标距离 default relativeStrength ** (1.0 / 3) * getAverageEdgeLength()
        double step;                        // 步长大小
        double initialStep;                 // 初始步长 default optimalDistance / 5.0
        int progress;                       // 正反馈系数，用于控制步长大小的更新 default 0 and progress < 5
        double energy0;                     // 初始能量
        double energy;                      // 当前能量
    public:
        YifanHu(const adj_list_t& g);
        double getAverageEdgeLength(std::vector<Point2D>& positions);
        void resetPropertiesValues(double stepRatio = 0.95, double relativeStrength = 0.2, int quadTreeMaxLevel = 20, double barnesHutTheta = 1.2, double convergenceThreshold = 1e-4);
        void updateStep();
        bool judge_converged();
        void operator()(std::vector<Point2D>& positions);
    };

}