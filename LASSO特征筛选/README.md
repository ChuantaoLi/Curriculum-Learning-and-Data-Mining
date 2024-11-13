### LassoFeatureSelector_main
这个是主函数文件，在实例化LassoFeatureSelector类时，需要传入下面这些参数：
1. input_train_data_path：输入训练集的路径
2. input_test_data_path：输入测试集的路径
3. output_train_path：输出训练集的路径
4. output_test_path：输出测试集的路径
5. Upper_limit_alpha：正则化搜索范围上限
6. Lower_limit_alpha：正则化搜索范围下限
7. iterations：LASSO回归迭代次数
8. cv：选择最佳正则化系数的交叉验证次数

实例化后调用总运行函数即可：lasso_selector.run_all()
### LassoFeatureSelector
这个是封装好的类，主要实现以下几个功能：
1. 计算特征筛选前后的方差膨胀因子，输出并导出
2. 绘制岭迹图并导出
3. 以MSE为损失函数进行LASSO回归
4. k折交叉验证进行最佳正则化系数的搜索
5. 导出特征筛选后的训练集和测试集
6. 无论输入的文件格式是xlsx文件还是csv文件，类都能读取

### 数据集
数据集来自网络入侵检测领域的经典数据集：NSLKDD

预处理好的数据集和导出的训练集测试集可以在百度网盘下载：

链接：https://pan.baidu.com/s/125SniuPOWFkrB4fONtIPQw?pwd=fgin 
提取码：fgin 

原始数据集见官网下载：

[ISCX NSL-KDD dataset 2009](https://www.unb.ca/cic/datasets/nsl.html)

### 导出的文件
1. LASSO系数矩阵.xlsx
2. LASSO回归岭迹图.png
3. 原始训练集的方差膨胀因子.xlsx
4. LASSO回归后训练集的方差膨胀因子.xlsx
5. NSLKDD_train_LASSO.xlsx
6. NSLKDD_test_LASSO.xlsx

### LASSO回归参数说明
若需要调整LASSO回归的参数，需要到LassoFeatureSelector文件的lasso_regression函数中进行修改

lassoreg = Lasso(alpha=alpha, max_iter=self.iterations, fit_intercept=True,precompute=False, copy_X=False,tol=0.0001, warm_start=False,positive=False,selection='cyclic')

alpha=alpha: 正则化强度，控制稀疏性

fit_intercept=True: 拟合截距

precompute=False: 是否预计算 Gram 矩阵，通常设置为 False

copy_X=True: 对输入数据进行复制

max_iter=self.iterations: 最大迭代次数，控制算法运行的最大迭代次数

tol=0.0001: 收敛的容忍度，指定算法收敛的阈值

warm_start=False: 如果为 True，则使用前一个调用的解决方案以适应的权重

positive=False: 如果为 True，则要求系数为正

selection='cyclic': 指定系数更新的策略。'cyclic' 表示按循环顺序逐个更新系数

