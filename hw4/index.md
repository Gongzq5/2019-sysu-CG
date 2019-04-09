# 计算机图形学 实验报告 HW4

*16340063 巩泽群*

**完整程序的示例如下：**

![完整程序过程](https://s2.ax1x.com/2019/04/09/AIfVbj.gif)



[TOC]

## Basic

### 画一个立方体(cube)：边长为4， 中心位置为(0, 0, 0)。

> 主要根据教程的 [变换](https://learnopengl-cn.github.io/01%20Getting%20started/07%20Transformations/) 以及 [坐标系统](https://learnopengl-cn.github.io/01%20Getting%20started/08%20Coordinate%20Systems/) 进行实验。

将程序从2D的平面图形改为3D的立体图形，首先要**修改顶点的坐标**，加入z轴对应的坐标值

```cpp
	std::vector<float> cubeVertices{
		// coordinate			// color
		 2.0f,  2.0f,  2.0f,    1.0f, 0.0f, 0.0f,
		 2.0f,  2.0f, -2.0f,    0.0f, 1.0f, 0.0f,
		 2.0f, -2.0f,  2.0f,	0.0f, 0.0f, 1.0f,
		 2.0f, -2.0f, -2.0f,	0.0f, 1.0f, 0.0f,
		-2.0f,  2.0f,  2.0f,	1.0f, 0.0f, 0.0f,
		-2.0f,  2.0f, -2.0f,	0.0f, 1.0f, 0.0f,
		-2.0f, -2.0f,  2.0f,	0.0f, 0.0f, 1.0f,
		-2.0f, -2.0f, -2.0f,	0.0f, 1.0f, 0.0f
	};
	
	std::vector<unsigned int> cubeIndices{
		0, 1, 2,    1, 2, 3,
		1, 3, 5,    3, 5, 7,
		0, 1, 5,    0, 4, 5,
		4, 5, 7,    4, 6, 7,
		0, 2, 4,    2, 4, 6,
		2, 3, 7,    2, 6, 7
	};
```

如上，这是一个立方体8个点的坐标，同时，根据按照三角形为图元绘制的顺序要求，加入了对应的 **EBO**，并生成并绑定对应的 **VAO、VBO、EBO** 。

然后要**修改GLSL** 的部分，为顶点着色器加入模型、观察、投影矩阵。

```cpp
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
```

片段着色器则没有太大的修改。

接下来我们要在**程序里声明并修改这三个矩阵的值**。

```cpp
// 获取 uniform 变量的位置
unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");

// 初始化为单位矩阵
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 projection = glm::mat4(1.0f);

// 修改三个矩阵的值为合适的值
model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
view = glm::translate(view, glm::vec3(0.0f, 0.0f, -15.0f));
projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

// 赋值给着色器程序中的 uniform 变量
glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
```



此时就可以看到一个3D的物体了。



### 分别启动和关闭深度测试 glEnable(GL_DEPTH_TEST) 、 glDisable(GL_DEPTH_TEST) ，查看区别，并分析原因。 

**关闭深度测试时**，会发现物体的几个面之间的遮挡关系发生了一定的错乱，这是因为后渲染的三角形片段会覆盖之前渲染在同一位置上的三角形片段的像素。如果位于后面的面渲染顺序比较靠后，那么就可能会遮挡到前面的立方体的面，因此会造成一定的错乱。

**打开深度测试时**，会发现物体的几个面的遮挡关系正常了，**OpenGL** 将所有片段的深度（`z`坐标值）保存在一个Z缓冲里。当打开深度测试时，如果一个后来的片段着色器的输出的深度值要比之前这个像素位置的片段的深度值要深（也就是说应该被之前的这个片段所遮挡），那么OpenGL便会丢弃这个后来的输出，也就达到了遮挡的效果。

> **官网的解释：**
>
> OpenGL存储它的所有深度信息于一个Z缓冲(Z-buffer)中，也被称为深度缓冲(Depth Buffer)。GLFW会自动为你生成这样一个缓冲（就像它也有一个颜色缓冲来存储输出图像的颜色）。深度值存储在每个片段里面（作为片段的**z**值），当片段想要输出它的颜色时，OpenGL会将它的深度值和z缓冲进行比较，如果当前的片段在其它片段之后，它将会被丢弃，否则将会覆盖。这个过程称为深度测试(Depth Testing)，它是由OpenGL自动完成的。



**示例如下：**

![关闭和打开GL_DEPTH_TEST的对比](https://s2.ax1x.com/2019/04/09/AIfFxS.gif)



### 平移(Translation)：使画好的cube沿着水平或垂直方向来回移动。 

更新在 **model** 矩阵上，在 **model** 矩阵的 **rotate** 变换之前加入 **translate** 变换，如下：

```cpp
// float direct[2] 的两个元素代表了水平和垂直方向的偏移，提供了一个 slider 进行选择
model = glm::translate(model, glm::vec3(direct[1], direct[0], 0.0f));
model = glm::rotate(...);
...
```

即可完成移动。

**示例如下：**

![平移示例](https://s2.ax1x.com/2019/04/09/AIfErQ.gif)



### 旋转(Rotation)：使画好的cube沿着XoZ平面的x=z轴持续旋转。

修改 **model** 矩阵，将旋转角度和时间关联，将旋转轴设置为 **(1, 0, 1)** 向量即可。

```cpp
model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1.0f, 0.0f, 1.0f));
```

**示例如下：**

![旋转示例](https://s2.ax1x.com/2019/04/09/AIfP8f.gif)



### 放缩(Scaling)：使画好的cube持续放大缩小。

修改 **model** 矩阵，将变化的倍数和时间关联，使用 **cos** 函数，可以看到立方体随着时间来回变动大小。

```cpp
float scales = cos((float)glfwGetTime()) + 1;
model = glm::scale(model, glm::vec3(scales, scales, scales));
```

**示例如下：**

![放缩示例](https://s2.ax1x.com/2019/04/09/AIfAKg.gif)



### 结合Shader谈谈对渲染管线的理解

图形渲染管线英文是**Graphics Pipeline**，他实际上是一个程序，这个程序从我们的输入数据开始进行渲染，经过一系列的过程，最终得到每个像素输出值，的程序。**OpenGL** 主要执行了两个部分，一个是将3D坐标投影到2D上，另一个是将2D坐标转换为每个像素的颜色值。

下面这张图大致显示了渲染管线的过程，从顶点着色器开始，然后经过几何着色器、片段着色器的渲染，以及一些列其他的过程，最终得到像素输出。

> 其中有一些我们已经学习过的过程，比如位于几何着色器和片段着色器之间的 **光栅化和插值** 的过程



![https://images0.cnblogs.com/blog/528205/201411/231935061878541.png](https://images0.cnblogs.com/blog/528205/201411/231935061878541.png)



## Bonus

实现一个 **正方体地球** 绕 **正方体太阳** 旋转的效果。 

实现时使用相同的VAO，分别乘不同的 model 矩阵，调整不同的位置和旋转效果，实现正方体地球和正方体太阳旋转的效果。



**示例如下：**

![地球绕太阳旋转](https://s2.ax1x.com/2019/04/09/AIfi28.gif)



