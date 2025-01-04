from statsmodels.stats.diagnostic import lilliefors
import matplotlib.pyplot as plt
import seaborn as sns

data = [0.23111172096338664, -1.0788096613719298, -0.35667494393873245, 0.5596157879354227, 3.9233585150918917,
        0.4382549309311553, -2.5257286443082556, -0.8675005677047231, -0.6931471805599453, 1.1631508098056809,
        -1.8971199848858813, -0.7133498878774648, -0.05129329438755058, -1.4271163556401458, 0.3148107398400336,
        -1.7719568419318752, 1.9430489167742813, -2.3025850929940455, -0.06187540371808753, -0.9675840262617056]

# Lilliefors 检验
statistic, p_value = lilliefors(data)

# 输出统计量和p值
print(f"Lilliefors 检验统计量: {statistic}")
print(f"Lilliefors 检验的 p 值: {p_value}")

# 绘制数据的直方图和正态分布拟合
sns.histplot(data, kde=True, stat='density', color='blue', label='Data Histogram')
plt.title('Data Distribution with Normal Distribution Fit')
plt.xlabel('Value')
plt.ylabel('Density')
plt.tight_layout()
plt.savefig(r'Lillefors test.png', dpi=600)
plt.show()
