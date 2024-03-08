#pragma once
#include "nodesoup.hpp"
#include <vector>

namespace nodesoup {
	// 在半径为1.0的圆上均匀分布顶点
	void circle(const adj_list_t& g, std::vector<Point2D>& positions);
	// 将顶点居中并缩放，使图形适应给定width * height大小的画布
	void center_and_scale(const adj_list_t& g, unsigned int width, unsigned int height, std::vector<Point2D>& positions);
}
