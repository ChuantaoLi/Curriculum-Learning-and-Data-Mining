import numpy as np
from scipy.stats import jarque_bera, gaussian_kde, skew, kurtosis
import matplotlib.pyplot as plt

# 原始数据
data = np.array(
    [1.26, 0.34, 0.70, 1.75, 50.57, 1.55, 0.08, 0.42, 0.50, 3.20, 0.15, 0.49, 0.95, 0.24, 1.37, 0.17, 6.98, 0.10, 0.94,
     0.38])

# 计算原始数据的Jarque-Bera检验、偏度和峰度
statistic_data, p_value_data = jarque_bera(data)
skewness_data = skew(data)
kurt_data = kurtosis(data, fisher=True)  # Fisher’s definition of kurtosis (normal ==> 0.0)

print(f"Original Data - Skewness: {skewness_data:.4f}, Kurtosis: {kurt_data:.4f}")

# 确保所有数据都是正值，然后对数据取自然对数
log_data = np.log(data)  # 过滤掉任何可能存在的非正值

# 计算对数变换后数据的Jarque-Bera检验、偏度和峰度
statistic_log_data, p_value_log_data = jarque_bera(log_data)
skewness_log_data = skew(log_data)
kurt_log_data = kurtosis(log_data, fisher=True)  # Fisher’s definition of kurtosis (normal ==> 0.0)

print(f"Log Transformed Data - Skewness: {skewness_log_data:.4f}, Kurtosis: {kurt_log_data:.4f}")

# 创建一个2x1的子图布局
fig, axes = plt.subplots(nrows=1, ncols=2, figsize=(14, 8.6))

# 绘制原始数据的概率密度函数
kde_data = gaussian_kde(data)
x_data = np.linspace(min(data), max(data), 1000)
axes[0].plot(x_data, kde_data(x_data), label='PDF')
axes[0].hist(data, density=True, alpha=0.5, bins=10)  # 添加直方图以对比PDF
axes[0].set_title(
    f'Original Data PDF - Jarque‐Bera Stat: {statistic_data:.4f}, P-value: {p_value_data:.4f}\nSkewness: {skewness_data:.4f}, Kurtosis: {kurt_data:.4f}')
axes[0].set_xlabel('Value')
axes[0].set_ylabel('Density')

# 绘制对数变换后数据的概率密度函数
kde_log_data = gaussian_kde(log_data)
x_log_data = np.linspace(min(log_data), max(log_data), 1000)
axes[1].plot(x_log_data, kde_log_data(x_log_data), label='PDF')
axes[1].hist(log_data, density=True, alpha=0.5, bins=10)  # 添加直方图以对比PDF
axes[1].set_title(
    f'Log Transformed Data PDF - Jarque‐Bera Stat: {statistic_log_data:.4f}, P-value: {p_value_log_data:.4f}\nSkewness: {skewness_log_data:.4f}, Kurtosis: {kurt_log_data:.4f}')
axes[1].set_xlabel('Log Value')
axes[1].set_ylabel('Density')

# 调整子图之间的间距
plt.tight_layout()

# 显示图表
plt.savefig(r'J-B test.png', dpi=600)
plt.show()
