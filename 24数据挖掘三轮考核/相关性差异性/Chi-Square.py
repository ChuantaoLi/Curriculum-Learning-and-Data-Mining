'''一般情况'''
import numpy as np
from scipy.stats import chi2_contingency

# 构建列联表
contingency_table = np.array([[10, 15, 20, 5], [8, 12, 18, 2]])

# 未校正的卡方检验
chi2_uncorrected, p_value_uncorrected, dof_uncorrected, expected_uncorrected = chi2_contingency(contingency_table)
print("卡方统计量:", chi2_uncorrected)
print("p值:", p_value_uncorrected)
print("自由度:", dof_uncorrected)
print("期望频数:", expected_uncorrected)

'''2X2'''
import numpy as np
from scipy.stats import chi2_contingency

# 构建一个2x2列联表
contingency_table = np.array([[10, 15], [8, 12]])
# 未校正的卡方检验
chi2, p_value, dof, expected = chi2_contingency(contingency_table)
print("未校正卡方统计量:", chi2)
print("未校正p值:", p_value)
# Yates校正卡方检验手动计算
a, b, c, d = contingency_table[0][0], contingency_table[0][1], contingency_table[1][0], contingency_table[1][1]
n = a + b + c + d
chi2_yates = ((np.abs(a * d - b * c) - n / 2) ** 2 * (a + b + c + d)) / ((a + b) * (c + d) * (a + c) * (b + d))
# 自由度对于2x2列联表为1
p_value_yates = 1 - np.sum(
    [np.exp(- chi2_yates / 2) * (chi2_yates / 2) ** k / np.math.factorial(k) for k in range(int(dof + 1))])
print("Yates校正卡方统计量:", chi2_yates)
print("Yates校正p值:", p_value_yates)
