import math
import pandas as pd

# 皮马印第安人糖尿病数据
train_data = pd.read_csv(r'train_data.csv').values
test_data = pd.read_csv(r'test_data.csv').values

# 划分连续和离散型变量
continuous_columns = [1, 2, 3, 4, 5, 6]  # Glucose, BloodPressure, SkinThickness, Insulin, BMI, DiabetesPedigreeFunction
discrete_columns = [0, 7]  # Pregnancies, Age

# 将数据按类别分离
separated_data = {0: [], 1: []}  # 创建一个空字典用来存储两个类别下的样本
for row in train_data:
    class_label = row[-1]  # 获取类别列
    separated_data[class_label].append(row)  # 把相同类别下的样本放到一起

# 分别计算两个类别下每个连续变量的均值和标准差、和离散变量的频率
class_summaries = {}  # 创建一个空字典用来存储两个类别下连续变量和离散变量的统计信息
for class_label, rows in separated_data.items():
    summaries = {}

    # 连续型变量的均值和标准差
    continuous_summaries = []
    for col_idx in continuous_columns:
        column = [row[col_idx] for row in rows]  # 把连续变量的一整列提取出来
        mean_val = sum(column) / len(column)  # 计算均值 μ_c
        variance = sum([(x - mean_val) ** 2 for x in column]) / len(column)  # 计算方差 σ_c^2
        stdev_val = math.sqrt(variance)  # 计算标准差 σ_c
        continuous_summaries.append((mean_val, stdev_val))

    # 离散型变量的频率计算
    discrete_summaries = []
    for col_idx in discrete_columns:
        column = [row[col_idx] for row in rows]  # 把离散变量的一整列提取出来
        value_counts = {val: column.count(val) for val in set(column)}  # 统计离散变量中每个取值出现的个数，加1是拉普拉斯修正
        total_count = len(rows)
        discrete_summaries.append((value_counts, total_count))

    # 存储两个类别下连续型和离散型的特征统计信息
    summaries['continuous'] = continuous_summaries
    summaries['discrete'] = discrete_summaries
    class_summaries[class_label] = summaries

# 拉普拉斯修正后的先验概率
class_prior = {}
total_samples = len(train_data)  # 样本长度 |D|
num_classes = len(class_summaries)  # 类别数 N

for class_label in separated_data:  # 取出两个类别的样本
    class_count = len(separated_data[class_label])  # 两个类别各自的样本数量 |D_c|
    class_prior[class_label] = (class_count + 1) / (total_samples + num_classes)

# 分类
predictions = []
for row in test_data:  # 对测试集的每一个样本进行分类
    probabilities = {}
    for class_label, summaries in class_summaries.items():  # 两个类别下连续变量和离散变量的统计信息

        # 将第一个类条件概率初始化为先验概率
        probabilities[class_label] = class_prior[class_label]

        # 连续变量的类条件概率
        for i, (mean_val, stdev_val) in enumerate(summaries['continuous']):
            x = row[continuous_columns[i]]  # 测试集每个样本中每个连续变量的取值
            exponent = math.exp(-((x - mean_val) ** 2 / (2 * stdev_val ** 2)))
            continous_probability = (1 / (math.sqrt(2 * math.pi) * stdev_val)) * exponent
            probabilities[class_label] *= continous_probability

        # 离散变量的类条件概率
        for i, (value_counts, total_count) in enumerate(summaries['discrete']):
            x = row[discrete_columns[i]]
            num_values = len(value_counts)  # 变量取值的数量 N_i
            discrete_probability = (value_counts.get(x, 0) + 1) / (total_count + num_values)  # 拉普拉斯修正
            probabilities[class_label] *= discrete_probability

    # 计算归一化因子 P(x)
    total_probability = sum(probabilities.values())
    for class_label in probabilities:
        probabilities[class_label] /= total_probability
    best_class = max(probabilities, key=probabilities.get)  # 看看哪个类别的后验概率大
    predictions.append(best_class)

# 计算准确率
correct_predictions = sum(1 for i in range(len(test_data)) if test_data[i][-1] == predictions[i])
accuracy = (correct_predictions / len(test_data)) * 100.0
print(f'code_accuracy: {accuracy:.4f}%')
print(probabilities)