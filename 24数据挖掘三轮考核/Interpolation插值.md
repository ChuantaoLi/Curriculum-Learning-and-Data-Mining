# Interpolation插值

​		对于缺失值的处理，比较常见的是数值分析中的插值和拟合这两种方法。插值指的是在离散数据的基础上补插连续函数，使得这条连续曲线通过**全部给定**的离散数据点；拟合则是找到一条“最优”的曲线，尽可能地**贴近**平面上一系列的点[1]。

​		设函数$y=f(x)$在区间$[a,b]$上有定义，且已知在点：
$$
a\le x_0<x_1<\cdots<x_n\le b\tag{1}
$$
上的值分别为：$y_0,y_1,\cdots ,y_n$，若存在一简单函数$P(x)$使得：
$$
P(x_i)=y_i,\quad (i=0,1,2,\cdots, n),\tag{2}
$$
则称$P(x)$为$f(x)$的插值函数，点$x_0,x_1,\cdots,x_n$称为插值节点，包含插值节点的区间$[a,b]$称为插值区间，求插值函数$P(x)$的方法称为插值法[2]。

​		设有$n+1$个互不相同的节点$(x_i,y_i),(i=0,1,2,\cdots,n)$，则存在唯一的多项式：
$$
L_n(x)=a_0+a_1x+a_2x^2+\cdots+a_nx^n,\tag{3}
$$
使得$L_n(x_j)=y_j,(j=0,1,2,\cdots,n)$。只要$n+1$个节点互异，满足上述条件的多项式是唯一存在的，如果不限制多项式的次数，插值多项式并不唯一，下面证明上述结论[2]：

​		构造如下方程组：
$$
\left\{ \begin{array}{l} a_0 + a_1 x_0 + a_2 x_0^2 + \ldots + a_n x_0^n = y_0 \\ a_0 + a_1 x_1 + a_2 x_1^2 + \ldots + a_n x_1^n = y_1 \\ \vdots \\ a_0 + a_1 x_n + a_2 x_n^2 + \ldots + a_n x_n^n = y_n \end{array} \right..\tag{4}
$$
​		令：
$$
\boldsymbol{A} =  \begin{bmatrix} 1 & x_0 & \cdots & x_0^n \\ 1 & x_1 & \cdots & x_1^n \\ \vdots & \vdots & \ddots & \vdots \\ 1 & x_n & \cdots & x_n^n \end{bmatrix}, \quad \boldsymbol{X} = 
\begin{bmatrix}
a_0 \\
a_1 \\
\vdots \\
a_n
\end{bmatrix},\quad
\boldsymbol{Y} = 
\begin{bmatrix}
y_0 \\
y_1 \\
\vdots \\
y_n
\end{bmatrix}.
$$
​		对于方程组组$\boldsymbol {A}\boldsymbol {X}=\boldsymbol{Y}$，由于矩阵$\boldsymbol{A}$是Vandermonde范德蒙矩阵，故其行列式[3]：
$$
|\boldsymbol{A}|=\prod_{i=1}^{n}\prod_{j=0}^{i-1}(x_i-x_j)\neq0.\tag{5}
$$
​		因此方程组$\boldsymbol {A}\boldsymbol {X}=\boldsymbol{Y}$有唯一解，从而公式(3)唯一存在。

### Lagrangian Polynomial Interpolation拉格朗日插值

​		拉格朗日插值的思想就是对于每个数据点都构造一个拉格朗日基函数$l_i(x)$，且对于数据点$x_j$满足以下性质：
$$
f_i(x_j)=
\left\{
\begin{array}{1}
1,& i=j\\
0,& i\neq j
\end{array}
\right.\tag{6}.
$$
​		假设有三个点：
$$
(x_0, y_0), (x_1, y_1), (x_2, y_2),
$$
则拉格朗日基函数可以写为：
$$
l_0(x) = \frac{(x - x_1)(x - x_2)}{(x_0 - x_1)(x_0 - x_2)},\\
l_1(x) = \frac{(x - x_0)(x - x_2)}{(x_1 - x_0)(x_1 - x_2)},\\
l_2(x) = \frac{(x - x_0)(x - x_1)}{(x_2 - x_0)(x_2 - x_1)}.\tag{7}
$$
​		因此可以构造插值多项式：
$$
L_2(x) = y_0 l_0(x) + y_1 l_1(x) + y_2 l_2(x).\tag{8}
$$
​		对于一般形式，拉格朗日基函数为[4]：
$$
l_i(x) = \frac{\prod_{j\neq i}(x-x_j)}{\prod_{j\neq i}(x_i-x_j)}.\tag{9}
$$
​		构造辅助函数：
$$
\omega_{n+1}(x) = (x - x_0)(x - x_1) \cdots (x - x_n).\tag{10}
$$

​		对于上式的求导可以选择取对数或使用乘法法则，这里选择取对数进行求导：
$$
\ln \omega_{n+1}(x)=\ln [(x-x_0)(x-x_1)\cdots (x-x_n)].\tag{11}
$$
​		然后对两边求导后可得：
$$
\omega'_{n+1}(x)=\omega_{n+1}(x)\sum_{i=0}^{n}\frac{1}{x-x_i}.\tag{12}
$$
​		展开可得：
$$
\omega'_{n+1}(x)=(x-x_1)\cdots(x-x_n)+(x-x_0)\cdots(x-x_n)+\cdots+(x-x_0)\cdots(x-x_{n-1}).\tag{13}
$$
​		代入具体的$x=x_k$最终可得：

$$
\omega'_{n+1}(x_k) = (x_k - x_0)(x_k - x_1) \cdots (x_k - x_{k-1})(x_k - x_{k+1}) \cdots (x_k - x_n).\tag{14}
$$

​		对于式(9)、式(10)和式(14)，可将拉格朗日基函数化简为如下形式：
$$
l_i(x)=\frac{\omega_{n+1}(x)}{(x-x_i)\omega'_{n+1}(x_i)}.\tag{15}
$$
​		拉格朗日插值多项式可化简为如下形式：
$$
L_n(x)=\sum_{k=0}^ny_k\frac{\omega_{n+1}(x)}{(x-x_i)\omega'_{n+1}(x_i)}.\tag{16}
$$
​		如果上面的推导过于抽象，那接下来以$n=2$为例带入上述式子计算：

​		对于$i=0$的情况：
$$
l_0(x) = \frac{(x - x_1)(x - x_2)}{(x_0 - x_1)(x_0 - x_2)},\tag{17}
$$

$$
\omega_{3}(x) = (x - x_0)(x - x_1)(x - x_2),\tag{18}
$$

$$
\omega'_{3}(x_0)=(x_0-x_1)(x_0-x_2)+(x_0-x_0)(x-x_1)+(x_0-x_0)(x_0-x_1)\\
=(x_0-x_1)(x_0-x_2)\tag{19}.
$$

​		可以观察到式(15)与式(16)成立。

### Runge Phenomenon龙格现象

​		在科学计算领域，龙格现象（Runge）指的是对于某些函数，使用均匀节点构造高次多项式差值时，在插值区间的边缘的误差可能很大的现象。它是由Runge在研究多项式差值的误差时发现的，这一发现很重要，因为它表明，并不是插值多项式的阶数越高，效果就会越好[5]。均匀分布节点插值在两端处波动极大，产生明显的震荡，均匀分布节点进行的插值就是一种拉格朗日插值的具体应用。在不熟悉曲线运动趋势的前提下，不要轻易使用高次插值。

​		同样属于多项式插值的还有牛顿插值[6]，尽管与拉格朗日插值法相比，牛顿插值法的计算过程具有继承性，但是牛顿插值也存在龙格现象的问题。拉格朗日插值和牛顿插值仅仅要求插值多项式在插值节点处与被插函数有相等的函数值，而这种插值多项式却不能全面反映被插值函数的性态。然而在许多实际问题中，不仅要求插值函数与被插值函数在所有节点处有相同的函数值，它也需要在一个或全部节点上插值多项式与被插函数有相同的低阶甚至高阶的导数值[7]。

​		所谓“低阶导数值相等”，指的是插值函数在节点附近的变化趋势与被插值函数更加接近，使得插值函数在节点处的切线与被插值函数的切线重合，从而在局部具有更好的逼近性；而“高阶导数值相等”指的是插值函数在节点处及其邻域内具有更好的逼近特性，确保对函数的光滑性要求。

### Hermite Interpolation埃尔米特插值

​		设函数$ f(x) $在区间$[a, b]$上有$ n+1 $个互异节点[2]：
$$
a = x_0 < x_1 < x_2 < \ldots < x_n = b.\tag{20}
$$
​		定义在$[a, b]$上函数$ f(x) $在节点上满足：
$$
f(x_i) = y_i, \quad f'(x_i) = y'_i \quad (i = 0, 1, 2, \ldots, n).\tag{21}
$$
​		可唯一确定一个次数不超过$ 2n + 1 $的多项式$ H_{2n+1}(x) = H(x) $满足： 
$$
H(x_j) = y_j, \quad H'(x_j) = m_j = y'_i \quad (j = 0, 1, \ldots, n), \tag{22}
$$
因为每个节点有2个条件（函数值和导数值），总共$2n+2$个条件，所以可以唯一确定一个次数不超过$2n+1$的多项式。

​		其余项为：
$$
R(x) = f(x) - H(x) = \frac{f^{(2n+2)}(\xi)}{(2n+2)!} \omega_{2n+2}(x),\tag{23}
$$
上式中，$f^{(2n+2)}(\xi)$表示原函数$f(x)$的$2n+2$阶导数在m某一点$\xi$处的值，其中$\xi\in(a,b)$。余项可以用来衡量插值精度或指导节点选择和插值方法优化。

$\omega_{2n+2}(x)$的定义如下：
$$
\omega_{2n+2}(x)=\prod_{i=0}^n(x-x_i)^2.\tag{24}
$$
​		下面以豆包给我的三个节点的示例演示埃尔米特插值：

​		设节点为$ x_0 = 0 $, $ x_1 = 1 $, $ x_2 = 2 $， 已知$ y_0 = f(x_0) = 0 $，$ y_1 = f(x_1) = 1 $ ，$ y_2 = f(x_2) = 4 $， $ y'_0 = f'(x_0) = 1 $，$ y'_1 = f'(x_1) = 3 $，$ y'_2 = f'(x_2) = 7 $。

​		现需要找一个次数不超过$ 2n + 1 = 5 $的多项式：
$$
H(x) = a_0 + a_1 x + a_2 x^2 + a_3 x^3 + a_4 x^4 + a_5 x^5.\tag{25}
$$
​		根据埃尔米特插值条件：
$$
H(x_0) = a_0 = 0,\\
H(x_1) = a_0 + a_1 + a_2 + a_3 + a_4 + a_5 = 1,\\
H(x_2) = a_0 + 2a_1 + 4a_2 + 8a_3 + 16a_4 + 32a_5 = 4 .\tag{26}
$$
​		对$ H(x) $求导得到：
$$
H'(x) = a_1 + 2a_2 x + 3a_3 x^2 + 4a_4 x^3 + 5a_5 x^4.\tag{27}
$$
​		可以求得：
$$
H'(x_0) = a_1 = 1,\\
H'(x_1) = a_1 + 2a_2 + 3a_3 + 4a_4 + 5a_5 = 3,\\
H'(x_2) = a_1 + 4a_2 + 12a_3 + 32a_4 + 80a_5 = 7.\tag{28}
$$
​		由$ a_0 = 0 $和$ a_1 = 1 $，将其代入其他方程：
$$
\begin{cases} 1 + a_2 + a_3 + a_4 + a_5 = 1 \\ 1 + 2a_2 + 4a_3 + 8a_4 + 16a_5 = 4 \\ 1 + 2a_2 + 3a_3 + 4a_4 + 5a_5 = 3 \\ 1 + 4a_2 + 12a_3 + 32a_4 + 80a_5 = 7 \end{cases} .\tag{29}
$$
​		化简得：
$$
\begin{cases} a_2 + a_3 + a_4 + a_5 = 0 \\ 2a_2 + 4a_3 + 8a_4 + 16a_5 = 3 \\ 2a_2 + 3a_3 + 4a_4 + 5a_5 = 2 \\ 4a_2 + 12a_3 + 32a_4 + 80a_5 = 6 \end{cases} .\tag{30}
$$
​		求解可得$ a_2 $，$ a_3 $，$ a_4 $，$ a_5 $的值，进而得到插值多项式$ H(x) $。

### Piecewise Cubic Hermite Interpolation分段三次埃尔米特插值

​		在给定一系列节点$ x_0 < x_1 < x_2 < \cdots < x_n $以及对应的函数值$ f(x_i) $ 和导数值 $ f'(x_i) $ ($ i = 0, 1, \cdots, n $)的基础上，不再像传统埃尔米特插值那样构建一个高次的全局多项式，而是把区间$[x_0, x_n]$划分成多个子区间$[x_i, x_{i+1}]$ ($ i = 0, 1, \cdots, n-1 $)，在每个子区间上分别构造一个三次多项式来进行插值，所以称为分段三次埃尔米特插值。

​		对于每个子区间$[x_i, x_{i+1}]$，设对应的三次多项式为$ H_i(x) $，其形式通常可以表示为：
$$
H_i(x) = a_{i0} + a_{i1}(x - x_i) + a_{i2}(x - x_i)^2 + a_{i3}(x - x_i)^3.\tag{31}
$$
​		这里让豆包给我绘制了不同插值方法的可视化对比：

![Interpolation Comparison](D:\24DataMiningAssess\Third round assessment\Interpolation Comparison.png)

```py
import numpy as np
import matplotlib.pyplot as plt
from scipy.interpolate import lagrange, CubicHermiteSpline


def f(x):
    return 1 / (1 + 25 * x ** 2)


def uniform_interpolation(x, n):
    x_nodes = np.linspace(-1, 1, n)
    y_nodes = f(x_nodes)
    return np.interp(x, x_nodes, y_nodes)


def lagrange_interpolation(x, n):
    x_nodes = np.linspace(-1, 1, n)
    y_nodes = f(x_nodes)
    poly = lagrange(x_nodes, y_nodes)
    return poly(x)


def hermite_interpolation(x, n):
    x_nodes = np.linspace(-1, 1, n)
    y_nodes = f(x_nodes)
    dy_nodes = np.gradient(y_nodes, x_nodes[1] - x_nodes[0])
    hermite = CubicHermiteSpline(x_nodes, y_nodes, dy_nodes)
    return hermite(x)


def cubic_hermite_interpolation(x, n):
    x_nodes = np.linspace(-1, 1, n)
    y_nodes = f(x_nodes)
    dy_nodes = np.gradient(y_nodes, x_nodes[1] - x_nodes[0])
    cubic_hermite = CubicHermiteSpline(x_nodes, y_nodes, dy_nodes)
    return cubic_hermite(x)


# 生成用于绘制函数的 x 值
x = np.linspace(-1, 1, 500)
y = f(x)

# 绘制原函数
plt.figure(figsize=(14, 8.6))
plt.plot(x, y, label='f(x)')

# 均匀节点分布插值
n = 10
y_uniform = uniform_interpolation(x, n)
plt.plot(x, y_uniform, label=f'Uniform Interpolation (n={n})')

# 拉格朗日插值
y_lagrange = lagrange_interpolation(x, n)
plt.plot(x, y_lagrange, label=f'Lagrange Interpolation (n={n})')

# 埃尔米特插值
y_hermite = hermite_interpolation(x, n)
plt.plot(x, y_hermite, label=f'Hermite Interpolation (n={n})')

# 三次埃尔米特插值
y_cubic_hermite = cubic_hermite_interpolation(x, n)
plt.plot(x, y_cubic_hermite, label=f'Cubic Hermite Interpolation (n={n})')

plt.legend()
plt.xlabel('x', fontsize=14)
plt.ylabel('y', fontsize=14)
plt.title('Interpolation Comparison', fontsize=16)
plt.savefig('Interpolation Comparison.png', dpi=600)
plt.show()

```

## 参考博客

1. [【数值分析——插值法】拉格朗日插值多项式及Matlab实现](https://blog.csdn.net/coollingomg/article/details/134192019?ops_request_misc=%257B%2522request%255Fid%2522%253A%252200ea6c77e5c48f2e41c47ca178415b0b%2522%252C%2522scm%2522%253A%252220140713.130102334..%2522%257D&amp;request_id=00ea6c77e5c48f2e41c47ca178415b0b&amp;biz_id=0&amp;utm_medium=distribute.pc_search_result.none-task-blog-2~all~sobaiduend~default-1-134192019-null-null.142^v100^pc_search_result_base7&amp;utm_term=%E6%8B%89%E6%A0%BC%E6%9C%97%E6%97%A5%E5%A4%9A%E9%A1%B9%E5%BC%8F&amp;spm=1018.2226.3001.4187)
2. [数值计算方法 华北理工大学_中国大学MOOC(慕课)](https://www.icourse163.org/course/NCST-1002988004?from=searchPage&outVendor=zw_mooc_pcssjg_)
3. [特殊矩阵(8)：Vandermonde 矩阵](https://blog.csdn.net/u011089523/article/details/72845136?fromshare=blogdetail&sharetype=blogdetail&sharerId=72845136&sharerefer=PC&sharesource=L040514&sharefrom=from_link)
4. [数值分析复习：Lagrange插值](https://blog.csdn.net/2301_76884115/article/details/136740874?fromshare=blogdetail&sharetype=blogdetail&sharerId=136740874&sharerefer=PC&sharesource=L040514&sharefrom=from_link)
5. [龙格现象(Runge Phenomenon)](https://zhuanlan.zhihu.com/p/138747068)
6. [如何直观、通俗地理解牛顿插值？ - 睿智君的回答 - 知乎](https://www.zhihu.com/question/346737214/answer/2938115648)
7. [【强烈推荐】清风：数学建模算法、编程和写作培训的视频课程以及Matlab等软件教学](https://www.bilibili.com/video/BV1DW411s7wi/?p=8&share_source=copy_web&vd_source=074552370fc3e89a4bdcaf33a8decdbc) 