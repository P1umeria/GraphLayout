#pragma once

#include <string>
#include <vector>

#include "vec2.hpp"


using vertex_t = unsigned int;

/**
 * ���ղ����б�ʾ����Ķ���
 */
struct node {
    vertex_t u;  /**< ��ı��r */
    vec2 pos;    /**< �ռ��е����� */
    float size;  /**< ��İ뾶 */
};

/**
 * ���ղ����д���ߵĶ���
 */
struct path {
    vertex_t from, to;          /**< ��Ӧ�ߵĶ˵�Ķ����ʶ�� */
    std::vector< vec2 > points; /**< ��ʾ�ߵ����ߵĿ��Ƶ� */
    bool bidirectional = false; /**< ��Ե�Ƿ�˫�� */
};

/**
 * ��������ͼ�β��ֵĲ���
 */
struct attributes {
    float node_size = 15;        /**< ����뾶 */
    float node_dist = 10;        /**< �����ڵ�߽�֮�����С���� */
    float layer_dist = 30;       /**< ������ͬ�㼶�ڵ�֮�����С�߽���� */
    float loop_angle = 55;       /**< �Ƕ�ȷ���˻����ӵ��ڵ��ϵ�λ�� */
    float loop_size = node_size; /**< ѭ���ӽڵ�����ľ���*/
};
