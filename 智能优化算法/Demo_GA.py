import numpy as np
import pandas as pd
from sklearn.neighbors import KNeighborsClassifier
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder  # 新增标签编码
from GA import jfs
import matplotlib.pyplot as plt

# 加载数据并转换标签
data = pd.read_csv('ionosphere.csv')
data = data.values
feat = np.asarray(data[:, 0:-1])
label = np.asarray(data[:, -1])

# 将标签转换为数值（例如 'g' 和 'b' 转为 0 和 1）
le = LabelEncoder()
label = le.fit_transform(label)

# 分割数据集
xtrain, xtest, ytrain, ytest = train_test_split(feat, label, test_size=0.3, stratify=label)
fold = {'xt': xtrain, 'yt': ytrain, 'xv': xtest, 'yv': ytest}

# 参数设置
opts = {'k': 5, 'fold': fold, 'N': 10, 'T': 200, 'CR': 0.8, 'MR': 0.01}

# 调用特征筛选函数
fmdl = jfs(xtrain, ytrain, opts)
sf = fmdl['sf']  # 特征选择索引
cv = fmdl['cv']  # 收敛曲线数组
nf = fmdl['nf']  # 选择的特征数量

# 使用筛选后的特征训练模型
x_train = xtrain[:, sf]
x_valid = xtest[:, sf]

mdl = KNeighborsClassifier(n_neighbors=5)
mdl.fit(x_train, ytrain)

# 计算准确率
y_pred = mdl.predict(x_valid)
Acc = np.sum(y_pred == ytest) / len(ytest)
print(f"Accuracy: {Acc * 100:.2f}%")
print(f"Selected Features: {fmdl['nf']}")

# 收敛曲线可视化
plt.figure()
plt.plot(fmdl['c'].ravel(), 'r-')
plt.xlabel('Iterations')
plt.ylabel('Fitness')
plt.title('GA Convergence')
plt.grid()
plt.show()
