import numpy as np
from scipy.stats import shapiro

data = np.array(
    [1.26, 0.34, 0.70, 1.75, 50.57, 1.55, 0.08, 0.42, 0.50, 3.20, 0.15, 0.49, 0.95, 0.24, 1.37, 0.17, 6.98, 0.10, 0.94,
     0.38])

# 进行 Shapiro-Wilk 检验
statistic, p_value = shapiro(data)

print(f"Shapiro-Wilk 统计量: {statistic}")
print(f"P 值: {p_value}")
