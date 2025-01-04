from matplotlib import rcParams

rcParams['font.family'] = 'Microsoft YaHei'
rcParams['axes.unicode_minus'] = False

import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
from scipy.stats import spearmanr

# 读取数据
df = pd.read_excel(r'八年级男生体测数据.xls')

# 计算斯皮尔曼相关系数和对应的p值矩阵
correlation_matrix, p_value_matrix = spearmanr(df)

# 将相关系数矩阵和p值矩阵转换为DataFrame，方便显示
correlation_df = pd.DataFrame(correlation_matrix, columns=df.columns, index=df.columns)
p_value_df = pd.DataFrame(p_value_matrix, columns=df.columns, index=df.columns)

# 绘制相关系数的热力图
plt.figure(figsize=(8, 8))
sns.heatmap(correlation_df, cmap='Blues', annot=True, fmt='.3f', vmin=-1, vmax=1)
plt.title('Spearman Correlation Heatmap of All Columns')
plt.tight_layout()
plt.savefig(r'Spearman_Correlation_Heatmap_of_All_Columns.png', dpi=600)
plt.show()

# 输出各列之间的p值
print("各列之间的 p 值矩阵：")
print(p_value_df)

# 你也可以将 p 值矩阵保存为 Excel 文件或 CSV 文件：
p_value_df.to_excel(r'Spearman_Correlation_p_values.xlsx', sheet_name='P-Values', index=True)
