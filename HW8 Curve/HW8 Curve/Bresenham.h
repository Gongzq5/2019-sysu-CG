#ifndef _BRESENHAM_H_
#define _BRESENHAM_H_

#include <iostream>
#include <functional>
#include <vector>
#include "mUtils.h"

namespace Bresenham {

using std::vector;
using mUtils::max;
using mUtils::swap2;
using mUtils::min;

#define Points vector<Point> 

struct Point {
	int x;
	int y;
	Point(int x_, int y_) : x(x_), y(y_) {}
};

// 将-100到100的数据整理到-1到1之间
vector<float> uniform(vector<float>& data) {
	for (auto& d : data) d /= 100;
	return data;
}

// 将point映射到float数组上
vector<float> pointsToFloat3(const Points &points) {
	vector<float> rv;
	for (auto point : points) {
		rv.push_back((float)point.x);
		rv.push_back((float)point.y);
		rv.push_back(0);
	}
	return uniform(rv);
}

// 生成线
vector<float> genLinePoints(Point from, Point to) {
	Points points;
	bool flipY = false, flipXY = false;

	int dx = abs(to.x - from.x);
	int dy = abs(to.y - from.y);

	if (from.x > to.x) {
		swap2(from, to);
	}
	if (from.y > to.y) {
		flipY = true;
		from.y = -from.y;
		to.y = -to.y;
	}
	if (dy > dx) {
		flipXY = true;
		swap2(from.x, from.y);
		swap2(to.x, to.y);
	}

	dx = to.x - from.x;
	dy = to.y - from.y;

	printf("%d, %d\n", dx, dy);

	// 意味着要使用 x 作为 +1 更新的坐标
	vector<int> p(dx, 2 * dy - dx);
	points.push_back(from);
	for (int i = 1; i < dx; i++) {
		if (p[i - 1] <= 0) {
			p[i] = p[i - 1] + 2 * dy;
		} else {
			p[i] = p[i - 1] + 2 * dy - 2 * dx;
		}
		if (p[i] > 0) {
			points.push_back(Point(from.x + i, points[i - 1].y + 1));
		} else {
			points.push_back(Point(from.x + i, points[i - 1].y));
		}
	}
	points.push_back(to);

	if (flipXY) {
		for (int i = 0; i < points.size(); i++) {
			swap2(points[i].x, points[i].y);
		}
	}
	if (flipY) {
		for (int i = 0; i < points.size(); i++) {
			points[i].y = -points[i].y;
		}
	}
	return pointsToFloat3(points);
}

// 生成三角形
vector<float> genTriPoints(Point p1, Point p2, Point p3) {
	vector<float> re = genLinePoints(p1, p2);
	
	vector<float> re2 = genLinePoints(p2, p3);
	re.insert(re.end(), re2.begin(), re2.end());

	vector<float> re3 = genLinePoints(p3, p1);
	re.insert(re.end(), re3.begin(), re3.end());
	
	return re;
}

// 圆的1/8对称映射
vector<Point> circle8(Point centre, int x, int y) {
	vector<Point> eightPoints = {
		Point(centre.x + x, centre.y + y),
		Point(centre.x - x, centre.y + y),
		Point(centre.x + x, centre.y - y),
		Point(centre.x - x, centre.y - y),
		Point(centre.x + y, centre.y + x),
		Point(centre.x - y, centre.y + x),
		Point(centre.x + y, centre.y - x),
		Point(centre.x - y, centre.y - x)
	};
	return eightPoints;
}

// 生成圆
vector<float> genCirclePositions(Point centre, int radius) {
	vector<Point> pv;
	int x = 0, y = radius, d = 3 - (2 * radius);
	auto eightPoints = circle8(centre, x, y);
	pv.insert(pv.end(), eightPoints.begin(), eightPoints.end());
	
	while (x < y) {
		if (d < 0) {
			d = d + 4 * x + 6;
		} else {
			d = d + 4 * (x - y) + 10;
			y--;
		}
		x++;
		auto eightPoints = circle8(centre, x, y);
		pv.insert(pv.end(), eightPoints.begin(), eightPoints.end());
	}

	auto data = pointsToFloat3(pv);
	return data;
}

std::function<bool(int x, int y)> genLineEquation(Point p1, Point p2, Point p3) {
	return [=](int x, int y) -> bool {
		auto lf = [&](int x, int y) {
			return (p1.y - p2.y) * x + (p2.x - p1.x) * y + (p1.x * p2.y - p2.x * p1.y);
		};
		if (lf(p3.x, p3.y) >= 0) {
			return lf(x, y) >= 0;
		} else {
			return lf(x, y) <= 0;
		}
	};
};

// 生成三角形，通过Edge Eqution算法
vector<float> genFilledTriPoints(Point p1, Point p2, Point p3) {
	vector<Point> points;
	int maxx = max(p1.x, p2.x, p3.x),
		maxy = max(p1.y, p2.y, p3.y),
		minx = min(p1.x, p2.x, p3.x),
		miny = min(p1.y, p2.y, p3.y);

	auto l1f = genLineEquation(p2, p1, p3);
	auto l2f = genLineEquation(p3, p2, p1);
	auto l3f = genLineEquation(p3, p1, p2);

	for (int i = minx; i < maxx; i++) {
		for (int j = miny; j < maxy; j++) {
			bool inside = l1f(i, j) && l2f(i, j) && l3f(i, j);
			if (inside) {
				points.push_back(Point(i, j));
			}
		}
	}
	printf("success rate: %f%%\n", (float)points.size() / ((maxx-minx)*(maxy-miny)));
	return pointsToFloat3(points);
}

// 绑定VAO
void pointsBindVAO(GLuint& VAO, GLuint& VBO, vector<float>& data) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}
}

#endif
