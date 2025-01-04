import numpy as np
import matplotlib.pyplot as plt
from scipy.stats import ks_2samp

# 定义两个样本
controlB = np.array(
    [1.26, 0.34, 0.70, 1.75, 50.57, 1.55, 0.08, 0.42, 0.50, 3.20, 0.15, 0.49, 0.95, 0.24, 1.37, 0.17, 6.98, 0.10, 0.94,
     0.38])
treatmentB = np.array(
    [2.37, 2.16, 14.82, 1.73, 41.04, 0.23, 1.32, 2.91, 39.41, 0.11, 27.44, 4.51, 0.51, 4.50, 0.18, 14.68, 4.66, 1.30,
     2.06, 1.19])

# 进行 K-S 检验
statistic, p_value = ks_2samp(controlB, treatmentB)

print(f"K-S 统计量: {statistic}")
print(f"P 值: {p_value}")


def empirical_cdf(data):
    # 对数据进行排序
    sorted_data = np.sort(data)
    # 计算每个数据点的经验累积分布函数值
    y = np.arange(1, len(sorted_data) + 1) / len(sorted_data)
    return sorted_data, y


def empirical_cdf_log(data):
    # 对数据取对数
    log_data = np.log(data)
    # 对取对数后的数据进行排序
    sorted_data = np.sort(log_data)
    # 计算每个数据点的经验累积分布函数值
    y = np.arange(1, len(sorted_data) + 1) / len(sorted_data)
    return sorted_data, y


# 计算 controlB 和 treatmentB 的经验分布函数
x_control, y_control = empirical_cdf(controlB)
x_treatment, y_treatment = empirical_cdf(treatmentB)

# 计算 controlB 和 treatmentB 取对数后的经验分布函数
x_control_log, y_control_log = empirical_cdf_log(controlB)
x_treatment_log, y_treatment_log = empirical_cdf_log(treatmentB)

# 创建两个子图
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 8.6))

# 第一个子图：原始数据的经验分布函数
ax1.plot(x_control, y_control, label='controlB')
ax1.plot(x_treatment, y_treatment, label='treatmentB')
ax1.set_xlabel('Value')
ax1.set_ylabel('Empirical CDF')
ax1.set_title('Empirical CDF of controlB and treatmentB')
ax1.legend()

# 第二个子图：取对数后数据的经验分布函数
ax2.plot(x_control_log, y_control_log, label='controlB (log)')
ax2.plot(x_treatment_log, y_treatment_log, label='treatmentB (log)')
ax2.set_xlabel('Log Value')
ax2.set_ylabel('Empirical CDF (log)')
ax2.set_title('Empirical CDF of log(controlB) and log(treatmentB)')
ax2.legend()

plt.tight_layout()
plt.savefig(r'K-S Empirical CDF.png', dpi=600)
plt.show()
