#pragma once
#include "nodesoup.hpp"
#include <vector>

namespace nodesoup {
	// �ڰ뾶Ϊ1.0��Բ�Ͼ��ȷֲ�����
	void circle(const adj_list_t& g, std::vector<Point2D>& positions);
	// ��������в����ţ�ʹͼ����Ӧ����width * height��С�Ļ���
	void center_and_scale(const adj_list_t& g, unsigned int width, unsigned int height, std::vector<Point2D>& positions);
}
