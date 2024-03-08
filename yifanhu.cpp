#include "yifanhu.hpp"
#include "algebra.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace nodesoup {

    using std::vector;

    // int quadtree::node_num = 0;
    bool quadtree::description = false;

    quadtree::quadtree(Point2D p, double sz, int max_dep)
        : origin(p)
        , size(sz)
        , isleaf(true)
        , max_dep(max_dep)
        , mass(0)
        , mass_center({ 0.0, 0.0 }) {
        children = new quadtree * [4];
        for (int i = 0; i < 4; ++i) {
            children[i] = nullptr;
        }
    }

    void quadtree::quad_divide() {
        if (max_dep == 0) {   // ����Ѿ������ָ��Σ������и������ԭ����
            if (children[0] == nullptr) {
                children[0] = this;
            }
        }
        else {              // ��һ������ָ�Ϊ�ĸ�������
            double children_size = size / 2.0;
            isleaf = false;
            // std::cout << "1\n";
            children[0] = new quadtree({ origin.x, origin.y + children_size }, children_size, max_dep - 1);
            children[1] = new quadtree({ origin.x + children_size, origin.y + children_size }, children_size, max_dep - 1);
            children[2] = new quadtree({ origin.x, origin.y }, children_size, max_dep - 1);
            children[3] = new quadtree({ origin.x + children_size, origin.y }, children_size, max_dep - 1);

        }
    }

    void quadtree::assimilateNode(Point2D p) {
        mass_center.x = mass * 1.0 / (mass + 1.0) * mass_center.x + p.x / (mass + 1.0);
        mass_center.y = mass * 1.0 / (mass + 1.0) * mass_center.y + p.y / (mass + 1.0);
        mass += 1;
    }

    bool quadtree::quad_insert(Point2D p) {
        if (origin.x <= p.x && origin.x + size > p.x && origin.y <= p.y && origin.y + size > p.y) {
            // ���������ɽ�����ڵ�,���ı�������������λ��
            assimilateNode(p);
            // node_num += 1;
            if (description) {
                printf("## (%.2f, %.2f) Successfully insert 1/4 of above part [sw(%.2f, %.2f), nw(%.2f, %.2f),ne(%.2f, %.2f),se(%.2f, %.2f)]\n",
                    p.x, p.y, origin.x, origin.y, origin.x, origin.y + size, origin.x + size, origin.y + size, origin.x + size, origin.y);
            }
            return true;
        }
        else {
            return false;
        }
    }

    bool quadtree::insert_node(Point2D p) {
        if (origin.x <= p.x && origin.x + size > p.x && origin.y <= p.y && origin.y + size > p.y) {
            // ������սڵ�Ĺ�������(ǰ) �������յĲ㼶�����յ��������ꡢ�������������
            if (description) {
                printf("The quad-part that will assimilate (%.2f, %.2f):", p.x, p.y);
                printf("Max-depth:%d  [sw(%.2f, %.2f), nw(%.2f, %.2f),ne(%.2f, %.2f),se(%.2f, %.2f)] Mess_center: (%.2f, %.2f)\n"
                    , max_dep, origin.x, origin.y, origin.x, origin.y + size, origin.x + size, origin.y + size, origin.x + size, origin.y, mass_center.x, mass_center.y);
            }
            // �Ȱ�ԭ���ı��棬�����Ҷ�ӽڵ�Ļ���Ҫ����һ����һ���ж�(һ���ڵ�ֻ��һ��������)
            Point2D ori_node = mass_center;
            // �ݹ����ɽ�����ڵ�,���ı�ÿ������������������λ��
            assimilateNode(p);
            // ������սڵ�Ĺ�������(��)
            if (description) {
                printf("After assimilating (%.2f, %.2f):", p.x, p.y);
                printf("Max-depth:%d  [sw(%.2f, %.2f), nw(%.2f, %.2f),ne(%.2f, %.2f),se(%.2f, %.2f)] Mess_center: (%.2f, %.2f)\n"
                    , max_dep, origin.x, origin.y, origin.x, origin.y + size, origin.x + size, origin.y + size, origin.x + size, origin.y, mass_center.x, mass_center.y);
            }
            if (mass == 1) {
                return true;
            }
            if (isleaf) {
                quad_divide();
                // std::cout << "2\n";
                // �²�����������ģ�Ҳ�����½ڵ������
                if (description) {
                    printf("New Children Mess-Center: (%.2f, %.2f)\n", mass_center.x, mass_center.y);
                }
                if (children[0] == this) {
                    return true;
                }
                // ��Ҫ���½ڵ���뵽��ǰ�ڵ㲻���ڵ�����
                bool ori_update = false, new_update = false;
                for (int i = 0; i < 4; ++i) {
                    if (children[i] == nullptr) {
                        // std::cout << 3 << std::endl;
                        break;
                    }
                    if (children[i]->origin.x <= ori_node.x && ori_node.x < children[i]->origin.x + children[i]->size
                        && children[i]->origin.y <= ori_node.y && ori_node.y < children[i]->origin.y + children[i]->size) {
                        // �������ڰ���֮ǰ�ĵ���ݹ��з��ٲ���(����δ�ﵽ������ȴ����һ���ڵ�������������)
                        if (description) {
                            printf("## Located at the same part -> Start recurisve divisions again to insert");
                        }
                        children[i]->quad_insert(ori_node);
                        if (description) {
                            printf("## Initialize this part by the existed node\n");
                        }
                        ori_update = true;
                        if (children[i]->origin.x <= p.x && p.x < children[i]->origin.x + children[i]->size && children[i]->origin.y <= p.y && p.y < children[i]->origin.y + children[i]->size) {
                            if (children[i]->insert_node(p)) {
                                return true;
                            }
                        }
                    }
                    if (children[i]->quad_insert(p)) {
                        new_update = true;
                    }
                    if (ori_update && new_update) {
                        return true;
                    }
                }
                // std::cout << ori_update << " " << new_update << "\n";
                // std::cout << ori_node.x << " " << ori_node.y << " " << origin.x << " " << origin.y << " " << size << "\n";
                // std::cout << "4\n";
            }
            else {
                for (int i = 0; i < 4; ++i) {
                    if (children[i] == nullptr) {
                        break;
                    }
                    if (children[i]->insert_node(p)) {
                        return true;
                    }
                }
                // std::cout << "5\n";
            }
        }
        else {
            return false;
        }
        std::cout << "exit 1\n";
    }

    void quadtree::get_farthest_mass(Point2D p, double threshold, vector<MassPoint>& pos_arr) {
        // ȥ������
        if (mass == 1 && fabs(p.x - mass_center.x) < 1e-8 && fabs(p.y - mass_center.y) < 1e-8) {
            return;
        }
        // ȥ��û�е���ڵĽڵ�
        if (mass == 0) {
            return;
        }
        // ����������С/���� С����ֵ����û�������ı�Ҫ(Զ����ĵ����Ӱ��Ƚ�С)��ֱ�Ӽ������ĸ������
        if (isleaf == false && size / sqrt((p.x - mass_center.x) * (p.x - mass_center.x) + (p.y - mass_center.y) * (p.y - mass_center.y)) < threshold) {
            // ����������������Ľڵ�������������
            pos_arr.push_back(MassPoint(mass_center, mass, size));
            return;
        }
        else if (isleaf && mass != 0) {
            // �����Ҷ�ӽڵ��Ҵ��ڵ�Ļ���˵���Ѿ�������������ľ�ͷ��ֱ�Ӽ������������(��Ψһ�ĵ�)�������
            pos_arr.push_back(MassPoint(mass_center, mass, size));
        }
        for (int i = 0; i < 4; ++i) {
            if (children[i] == nullptr || children[i] == this) {
                return;
            }
            children[i]->get_farthest_mass(p, threshold, pos_arr);
        }
    }

    void quadtree::get_all_mass_center(std::vector<MassPoint>& center) {
        for (int i = 0; i < 4; ++i) {
            if (children[i] == nullptr || children[i] == this) {
                return;
            }
            if (fabs(children[i]->mass_center.x) > 1e-6 || fabs(children[i]->mass_center.y) > 1e-6) {
                center.push_back(MassPoint(children[i]->mass_center, children[i]->mass, children[i]->size));
                children[i]->get_all_mass_center(center);
            }
        }
    }

    void quadtree::cal_electrical_forces(Point2D p, double threshold, double c, double k, Point2D& e_force) {
        // ȥ������
        if (mass == 1 && fabs(p.x - mass_center.x) < 1e-8 && fabs(p.y - mass_center.y) < 1e-8) {
            return;
        }
        // ȥ��û�е���ڵĽڵ�
        if (mass == 0) {
            return;
        }
        // ����������С/����С����ֵ����û�������ı�Ҫ��ֱ�Ӽ������ĸ������
        if (isleaf == false && size / (p - mass_center).norm() < threshold) {
            if ((p - mass_center).norm() > 1000.0) {
                return;
            }
            double distance_sqr = (p.x - mass_center.x) * (p.x - mass_center.x) + (p.y - mass_center.y) * (p.y - mass_center.y);
            // fr(i,j) = -C*K**2 / ||xi - xj||
            // �����չ�ʽ����fr(i,j)ȡ������ô����Ϊ ʩ����node(j) - ������node(i)
            // ������ʹ��������������ʩ���������Ĳ� node(i) - node(j)�����Դ˴�fr(i,j)ȡ��
            // ���Ĵ�С�����ĵĴ�Сself.mass������
            e_force.x += (p.x - mass_center.x) * (c * k * k / distance_sqr) * mass;
            e_force.y += (p.y - mass_center.y) * (c * k * k / distance_sqr) * mass;
            return;
        }
        else if (isleaf && mass != 0) {
            if ((p - mass_center).norm() > 1000.0) {
                return;
            }
            // �����Ҷ�ӽڵ��Ҵ��ڵ�Ļ���˵���Ѿ�������������ľ�ͷ��ֱ�Ӽ������������(��Ψһ�ĵ�)�������
            double distance_sqr = (p.x - mass_center.x) * (p.x - mass_center.x) + (p.y - mass_center.y) * (p.y - mass_center.y);
            e_force.x += (p.x - mass_center.x) * (c * k * k / distance_sqr) * mass;
            e_force.y += (p.y - mass_center.y) * (c * k * k / distance_sqr) * mass;
            return;
        }
        // �ﵽ���ָ���������򣬲��ٱ�������
        for (int i = 0; i < 4; ++i) {
            if (children[i] == nullptr || children[i] == this) {
                return;
            }
            children[i]->cal_electrical_forces(p, threshold, c, k, e_force);
        }
    }

    quadtree::~quadtree() {
        for (int i = 0; i < 4; ++i) {
            if (children[i] == nullptr || children[i] == this) {
                break;
            }
            delete children[i];
        }
        delete children;
    }

    YifanHu::YifanHu(const adj_list_t& g) : g_(g) {
        // ��ʼ�����в���������GephiԴ������,���бȽϿ�����Ҫ�Ķ�����relativeStrength����optimalDistance���
        progress = 0;
        energy = pow(2, 31) - 1;  // ��ʼ����Ĭ��Ϊ�����
        stepRatio = 0.95;
        relativeStrength = 0.2;
        quadTreeMaxLevel = 20;
        barnesHutTheta = 1.2;
        adaptiveCooling = true;
        convergenceThreshold = 1e-4;
        optimalDistance = 0.0;
        AverageEdgeLength = 0.0;
        initialStep = optimalDistance / 5.0;
        step = initialStep;
        converged = false;
    }

    double YifanHu::getAverageEdgeLength(std::vector<Point2D>& positions) {
        node_num = positions.size();
        double num = 0.0;
        AverageEdgeLength = 0.0;
        for (int i = 0; i < g_.size(); ++i) {
            for (int j = 0; j < g_[i].size(); ++j) {
                if (g_[i][j] <= i) {
                    continue;
                }
                Vector2D delta = positions[i] - positions[g_[i][j]];
                double distance = delta.norm();
                AverageEdgeLength = AverageEdgeLength * (num / (num + 1.0)) + distance * (1.0 / (num + 1.0));
                // std::cout << "getAverage: " << distance << " " << AverageEdgeLength << std::endl;
                num += 1.0;
            }
        }
        return AverageEdgeLength;
    }

    void YifanHu::resetPropertiesValues(double stepRatio, double relativeStrength, int quadTreeMaxLevel, double barnesHutTheta, double convergenceThreshold) {
        this->stepRatio = stepRatio;
        this->relativeStrength = relativeStrength;
        this->quadTreeMaxLevel = quadTreeMaxLevel;
        this->barnesHutTheta = barnesHutTheta;
        this->convergenceThreshold = convergenceThreshold;
        this->optimalDistance = pow(relativeStrength, 1.0 / 3) * AverageEdgeLength;
        // std::cout << optimalDistance << " " << relativeStrength << " " << AverageEdgeLength << std::endl;
        this->initialStep = optimalDistance / 5.0;
        this->step = initialStep;
    }

    void YifanHu::updateStep() {
        if (adaptiveCooling) {
            if (energy < energy0) {
                progress += 1;
                if (progress >= 5) {
                    progress = 0;
                    step /= stepRatio;
                }
            }
            else {
                progress = 0;
            }
        }
        else {
            step /= stepRatio;
        }
    }

    bool YifanHu::judge_converged() {
        return converged;
    }

    void YifanHu::operator()(std::vector<Point2D>& positions) {
        if (positions.size() == 0) {
            return;
        }
        double min_x = positions[0].x, max_x = min_x;
        double min_y = positions[0].y, max_y = min_y;
        for (int i = 1; i < positions.size(); ++i) {
            if (positions[i].x < min_x) {
                min_x = positions[i].x;
            }
            if (positions[i].y < min_y) {
                min_y = positions[i].y;
            }
            if (positions[i].x > max_x) {
                max_x = positions[i].x;
            }
            if (positions[i].y > max_y) {
                max_y = positions[i].y;
            }
            // std::cout << "ori: " << positions[i].x << " " << positions[i].y << std::endl;
        }
        double size = max_x - min_x > max_y - min_y ? max_x - min_x : max_y - min_y;
        quadtree* BH_tree = new quadtree({ min_x - 1e-5, min_y - 1e-5 }, size + 2e-5, quadTreeMaxLevel);
        for (int i = 0; i < positions.size(); ++i) {
            // printf("%f %f %f %f %f\n", min_x, min_y, size, positions[i].x, positions[i].y);
            BH_tree->insert_node(positions[i]);
        }
        vector<Vector2D> e_forces_move;
        // ����ÿһ�������ܵ��ĳ���ƫ������delta(������-ʩ�����λ��ƫ����)�Ѱ�����BH�㷨�������
        for (int i = 0; i < positions.size(); ++i) {
            Point2D e_force_vector = { 0.0, 0.0 };
            BH_tree->cal_electrical_forces(positions[i], barnesHutTheta, relativeStrength, optimalDistance, e_force_vector);
            e_forces_move.push_back({ e_force_vector.x, e_force_vector.y });
            // std::cout << "ef: " << relativeStrength << " " << optimalDistance << std::endl;
        }
        // s1�����ڵ����ӽڵ�����������spring�����Ĵ�С
        // s2�����ӽڵ��ܸ��ڵ�����������spring�����Ĵ�С
        double s1 = 1.0;
        double s2 = 1.0;
        vector<Vector2D> spring_forces_move(node_num, { 0.0, 0.0 });
        for (int i = 0; i < g_.size(); ++i) {
            for (int j = 0; j < g_[i].size(); ++j) {
                if (g_[i][j] <= i) {
                    continue;
                }
                double distance = (positions[i] - positions[g_[i][j]]).norm();
                spring_forces_move[i] += s1 * (positions[i] - positions[g_[i][j]]) * distance / optimalDistance;
                spring_forces_move[g_[i][j]] -= s2 * (positions[i] - positions[g_[i][j]]) * distance / optimalDistance;
                // std::cout << optimalDistance << " " << distance << std::endl;
            }
        }
        // �˴�ȡelectric_forces_move - spring_forces_move����ӦdeltaΪ������-ʩ�����λ��ƫ����
        // ����-����
        for (int i = 0; i < e_forces_move.size(); ++i) {
            // std::cout << "sp: " << spring_forces_move[i].dx << " " << spring_forces_move[i].dy << std::endl;
            e_forces_move[i] -= spring_forces_move[i];
        }
        energy0 = energy;
        // ������ʸ���Ĵ�С�ܺͼ������ֵ
        double max_force = 1.0;
        energy = 0.0;
        for (int i = 0; i < e_forces_move.size(); ++i) {
            double force = e_forces_move[i].norm();
            if (force < 0.01) {
                energy += 0.01;
            }
            else {
                energy += force;
            }
            if (max_force < force) {
                max_force = force;
            }
        }
        double rate = step / max_force;
        for (int i = 0; i < e_forces_move.size(); ++i) {
            positions[i] += e_forces_move[i] * rate;
            // positions[i] += e_forces_move[i] / e_forces_move[i].norm();
        }
        updateStep();
        if (fabs(energy - energy0) / energy < convergenceThreshold) {
            converged = true;
        }
        delete BH_tree;
    }

}