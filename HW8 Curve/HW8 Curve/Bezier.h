#pragma once
#ifndef _BEZIER_H_
#define _BEZIER_H_

#include <iostream>
#include <glad/glad.h>
#include <functional>
#include <vector>
#include "mUtils.h"
#include <windows.h>

namespace Bezier {

	using std::vector;
	using mUtils::max;
	using mUtils::swap2;
	using mUtils::min;

	float deltaT = 0.001f;

#define Points vector<Point> 

	struct Point {
		float x;
		float y;
		Point(float x_, float y_) : x(x_), y(y_) {}

		Point operator +(const Point& other) {
			return Point(this->x + other.x, this->y + other.y);
		}

		Point operator -(const Point& other) {
			return Point(x - other.x, y - other.y);
		}

		Point operator *(int scale) {
			return Point(x * scale, y * scale);
		}
	};

	// 将point映射到float数组上
	vector<float> pointsToFloat3(const Points &points) {
		vector<float> rv;
		for (auto point : points) {
			rv.push_back(point.x);
			rv.push_back(point.y);
			rv.push_back(0.0f);
		}
		return rv;
	}

	float bernstein(int i, int n, float t) {
		int tmp1 = 1, tmp2 = 1, tmp3 = 1;
		for (int j = i + 1; j <= n; j++) tmp1 *= j;
		for (int j = 1; j <= n - i; j++) tmp2 *= j;
		return ((float)tmp1 / (float)tmp2) * pow(t, i) * pow(1 - t, n - i);
	}

	// 1阶Bizier曲线，即直线
	vector<float> genBezierCurvePoints(Point p1, Point p2, float tLimit = 1.0f) {
		vector<Point> points;

		for (float t = 0; t < 1; t += deltaT) {
			float x = bernstein(0, 1, t) * p1.x + bernstein(1, 1, t) * p2.x;
			float y = bernstein(0, 1, t) * p1.y + bernstein(1, 1, t) * p2.y;
			points.push_back(Point(x, y));
		}

		return pointsToFloat3(points);
	}

	// 2阶Bizier曲线，即直线
	vector<float> genBezierCurvePoints(Point p1, Point p2, Point p3) {
		vector<Point> points;

		for (float t = 0; t < 1; t += deltaT) {
			float x = bernstein(0, 2, t) * p1.x + bernstein(1, 2, t) * p2.x + bernstein(2, 2, t) * p3.x;
			float y = bernstein(0, 2, t) * p1.y + bernstein(1, 2, t) * p2.y + bernstein(2, 2, t) * p3.y;
			points.push_back(Point(x, y));
		}

		return pointsToFloat3(points);
	}

	// 任意阶的Bezier曲线
	vector<float> genBezierCurvePoints(vector<Point> ps, float tLimit=1.0f) {
		int n = ps.size() - 1; // n + 1 个顶点
		//if (n>2) n = 2;
		vector<Point> points;

		for (float t = 0; t < tLimit; t += deltaT) {
			Point next(0, 0);
			for (int i = 0; i < n + 1; i++) { // 0 到 n 表示 n + 1 个顶点
				float param = bernstein(i, n, t);
				next.x += ps[i].x * param;
				next.y += ps[i].y * param;
			}
			points.push_back(next);
		}

		return pointsToFloat3(points);
	}

	Point genPointPercentage(Point p1, Point p2, float tLimit=1.0f) {
		float x = (p2.x - p1.x) * tLimit + p1.x;
		float y = (p2.y - p1.y) * tLimit + p1.y;
		return Point(x, y);
	}

	vector<float> genAssistantLinePoints(vector<Point> ps, float tLimit = 1.0f) {
		int n = ps.size() - 1;
		vector<Point> points, tPoints;
		vector<float> pointsF;
		// 直接相连的线
		for (int i = 0; i < n; i++) {
			vector<float> tmp = genBezierCurvePoints(ps[i], ps[i + 1]);
			pointsF.insert(pointsF.end(), tmp.begin(), tmp.end());
		}
		
		tPoints.assign(ps.begin(), ps.end());
		
		for (int j = 0; j < n; j++) {
			points.clear();

			points.push_back(genPointPercentage(tPoints[0], tPoints[1], tLimit));

			for (int i = 0; i < tPoints.size() - 2; i++) {
				Point p1 = genPointPercentage(tPoints[i], tPoints[i + 1], tLimit);
				Point p2 = genPointPercentage(tPoints[i + 1], tPoints[i + 2], tLimit);
				vector<float> tmp = genBezierCurvePoints(p1, p2);
				pointsF.insert(pointsF.end(), tmp.begin(), tmp.end());
				
				points.push_back(p2);
			}
			tPoints.clear();
			tPoints.assign(points.begin(), points.end());
		}

		return pointsF;
	}

	vector<float> genAssistantPoints(vector<Point> ps, float tLimit = 1.0f) {
		int n = ps.size() - 1; 
		vector<Point> points;

		// TODO
		// 
		// 
		//
		//

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
