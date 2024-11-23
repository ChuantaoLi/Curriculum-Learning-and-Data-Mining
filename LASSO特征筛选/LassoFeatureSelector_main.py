from LassoFeatureSelector import *
'''初始化'''
input_train_data_path = r'D:\贫困论文\Sichuan_train.xlsx'  # 输入训练集的路径
input_val_data_path = r'D:\贫困论文\Sichuan_val.xlsx'  # 输入验证集的路径
output_train_path = r'D:\贫困论文\Sichuan_train_LASSO.xlsx'  # 输出训练集的路径
output_val_path = r'D:\贫困论文\Sichuan_val_LASSO.xlsx'  # 输出验证集的路径
input_test_data_path = r'D:\贫困论文\Sichuan_test.xlsx'  # 输入测试集的路径
output_test_path = r'D:\贫困论文\Sichuan_test_LASSO.xlsx'  # 输出测试集的路径
upper_limit_alpha = 0.001  # 正则化搜索范围上限
lower_limit_alpha = 0.25  # 正则化搜索范围下限
iterations = 500  # LASSO回归迭代次数
cv = 10  # 选择最佳正则化系数的交叉验证次数

# 调用函数
lasso_selector = LassoFeatureSelector(input_train_data_path, input_val_data_path, output_train_path, output_val_path,
                                      input_test_data_path, output_test_path, upper_limit_alpha, lower_limit_alpha,
                                      iterations, cv)
lasso_selector.run_all()
