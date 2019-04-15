#ifndef _HW4_H_
#define _HW4_H_

#include <iostream>
#include <functional>
#include <vector>
#include <glad/glad.h>
#include "mUtils.h"

namespace HW4 {

	using std::vector;
	using mUtils::max;
	using mUtils::swap2;
	using mUtils::min;

	

	// °ó¶¨VAO
	void bind(vector<float>& data, GLuint& VAO, GLuint& VBO) {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), data.data(), GL_DYNAMIC_DRAW);		
	}

	// °ó¶¨VAO
	void bindEBO(vector<unsigned int>& indices, GLuint& VAO, GLuint& EBO) {
		glGenBuffers(1, &EBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
	}
}

#endif
