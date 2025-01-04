import numpy as np
from scipy import stats
from statsmodels.stats.diagnostic import lilliefors
import matplotlib.pyplot as plt
import seaborn as sns

# 数据
data = [0.23111172096338664, -1.0788096613719298, -0.35667494393873245, 0.5596157879354227, 3.9233585150918917,
        0.4382549309311553, -2.5257286443082556, -0.8675005677047231, -0.6931471805599453, 1.1631508098056809,
        -1.8971199848858813, -0.7133498878774648, -0.05129329438755058, -1.4271163556401458, 0.3148107398400336,
        -1.7719568419318752, 1.9430489167742813, -2.3025850929940455, -0.06187540371808753, -0.9675840262617056]

# 1. **Jarque-Bera (J-B) 检验**
jb_statistic, jb_p_value = stats.jarque_bera(data)
print(f"Jarque-Bera 检验统计量: {jb_statistic:.3f}, p 值: {jb_p_value:.3f}")

# 2. **Shapiro-Wilk (S-W) 检验**
sw_statistic, sw_p_value = stats.shapiro(data)
print(f"Shapiro-Wilk 检验统计量: {sw_statistic:.3f}, p 值: {sw_p_value:.3f}")

# 3. **Kolmogorov-Smirnov (K-S) 检验**
# 假设数据符合正态分布，使用均值和标准差作为参数来进行检验
ks_statistic, ks_p_value = stats.kstest(data, 'norm', args=(np.mean(data), np.std(data)))
print(f"Kolmogorov-Smirnov 检验统计量: {ks_statistic:.3f}, p 值: {ks_p_value:.3f}")

# 4. **Lilliefors 检验**
lilliefors_statistic, lilliefors_p_value = lilliefors(data)
print(f"Lilliefors 检验统计量: {lilliefors_statistic:.3f}, p 值: {lilliefors_p_value:.3f}")

# 5. **Anderson-Darling (A-D) 检验**
ad_result = stats.anderson(data, dist='norm')
print(f"Anderson-Darling 检验统计量: {ad_result.statistic:.3f}")

# A-D 检验的临界值和对应的显著性水平
print("Anderson-Darling 检验临界值：")
for i in range(len(ad_result.critical_values)):
    print(f"{ad_result.significance_level[i]}% 级别的临界值: {ad_result.critical_values[i]:.3f}")

# 可视化数据的直方图和正态分布拟合
sns.histplot(data, kde=True, stat='density', color='blue', label='Data Histogram')
plt.title('Data Distribution with Normal Distribution Fit')
plt.xlabel('Value')
plt.ylabel('Density')
plt.show()
