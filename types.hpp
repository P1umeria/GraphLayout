#pragma once

#include <string>
#include <vector>

#include "vec2.hpp"


using vertex_t = unsigned int;

/**
 * 最终布局中表示顶点的对象
 */
struct node {
    vertex_t u;  /**< 点的标号r */
    vec2 pos;    /**< 空间中的坐标 */
    float size;  /**< 点的半径 */
};

/**
 * 最终布局中代表边的对象
 */
struct path {
    vertex_t from, to;          /**< 对应边的端点的顶点标识符 */
    std::vector< vec2 > points; /**< 表示边的折线的控制点 */
    bool bidirectional = false; /**< 边缘是否双向 */
};

/**
 * 包含所需图形布局的参数
 */
struct attributes {
    float node_size = 15;        /**< 顶点半径 */
    float node_dist = 10;        /**< 两个节点边界之间的最小距离 */
    float layer_dist = 30;       /**< 两个不同层级节点之间的最小边界距离 */
    float loop_angle = 55;       /**< 角度确定了环连接到节点上的位置 */
    float loop_size = node_size; /**< 循环从节点延伸的距离*/
};
