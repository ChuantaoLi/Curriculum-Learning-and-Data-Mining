import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import norm

# 创建一个新的图形
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 8.6))

# 第一个子图：不同偏度下的正态分布
# 实际上，正态分布的偏度为 0，这里我们通过改变均值来模拟不同的“偏度”效果
means = [-2, 0, 2]
x = np.linspace(-10, 10, 1000)
for mean in means:
    # 标准差为 1，均值不同
    y = norm.pdf(x, loc=mean, scale=1)
    ax1.plot(x, y, label=f'mean={mean}')
ax1.set_title('Probability Density Function (PDF) with Different Skewnesses')
ax1.set_xlabel('x')
ax1.set_ylabel('PDF')
ax1.legend()

# 第二个子图：不同峰度下的正态分布
# 实际上，正态分布的峰度为 3，这里我们通过改变标准差来模拟不同的“峰度”效果
stds = [0.5, 1, 2]
x = np.linspace(-10, 10, 1000)
for std in stds:
    # 均值为 0，标准差不同
    y = norm.pdf(x, loc=0, scale=std)
    ax2.plot(x, y, label=f'std={std}')
ax2.set_title('Probability Density Function (PDF) with Different Standard Kurtoses')
ax2.set_xlabel('x')
ax2.set_ylabel('PDF')
ax2.legend()

plt.savefig(r'skewness_and_kurtosis.png', dpi=600)
plt.show()
