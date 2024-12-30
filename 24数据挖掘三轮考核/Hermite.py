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
