import pandas as pd
from PyEMD import EMD
import matplotlib.pyplot as plt
from matplotlib import rcParams
import numpy as np
import pmdarima as pm
import warnings

rcParams['font.family'] = 'sans-serif'
rcParams['font.sans-serif'] = ['Microsoft YaHei']

'''需要调整的地方'''
initial_data = pd.read_excel(r'D:\0海大数模校赛\initial_data.xlsx')  # 数据集路径
threshold_index = [18, 18]  # 奇异值个数
windowLen = 30  # 嵌入窗口长度

'''经验模态分解'''
initial_data['Date'] = pd.to_datetime(initial_data['Date'], format='%Y年%m月')
initial_data.set_index('Date', inplace=True)
initial_data.sort_index(inplace=True)  # 检查时间序列是否升序
signal = initial_data['Value'].values  # 时间序列因变量为signal

emd = EMD()  # 经验模态分解
IMFs = emd.emd(signal)

fig, axs = plt.subplots(len(IMFs) + 1, 1, figsize=(16, 3 * (len(IMFs) + 1)))
imf_data = pd.DataFrame(index=initial_data.index)
for i, imf in enumerate(IMFs):
    axs[i].plot(initial_data.index, imf, linewidth=4)
    axs[i].set_title(f"IMF {i + 1}", fontsize=14)
    axs[i].grid(True)
    imf_data[f'IMF_{i + 1}'] = imf

residue = signal - sum(IMFs)
axs[len(IMFs)].plot(initial_data.index, residue, linewidth=3)
axs[len(IMFs)].set_title("Residue", fontsize=14)
axs[len(IMFs)].grid(True)
imf_data['Residue'] = residue

imf_data.to_excel('经验模态分解结果.xlsx')  # 保存固有模态函数和残差序列

plt.subplots_adjust(hspace=0.4)
plt.savefig(r'经验模态分解可视化.png', dpi=600)
# plt.show()

'''拟合初始信号'''
# 移动平均法平滑初始信号
window_size = 2  # 窗口大小
initial_data['Smoothed_Value'] = initial_data['Value'].rolling(window=window_size).mean()

# 使用pmdarima自动寻找基于平滑后的数据的最佳ARIMA参数
model = pm.auto_arima(initial_data['Smoothed_Value'].dropna(),
                      start_p=1, start_q=1,
                      max_p=5, max_q=5,
                      d=None, max_d=2,
                      seasonal=False,
                      trace=False,
                      error_action='ignore',
                      suppress_warnings=True,
                      stepwise=True,
                      n_fits=50)

# 打印模型摘要
# print(model.order)
print(model.summary())
print('\n')

# 拟合数据
initial_fitted = model.predict_in_sample()
initial_fitted.to_excel(r'初始信号拟合结果.xlsx')

# 模型摘要
summary_initial = pd.DataFrame(model.summary().tables[1].data[1:], columns=model.summary().tables[1].data[0])
# summary_initial.to_excel(r'拟合初始信号的模型摘要.xlsx', index=False)

'''拟合固有模态函数'''
imf_data = pd.read_excel(r'经验模态分解结果.xlsx')  # 固有模态函数和残差，但残差往往很小，因此下面不拟合残差
imf_fitted_results = pd.DataFrame()  # 存储固有模态函数的拟合结果

# 存储每条IMF的ARIMA模型参数
model_params = []

# 对每一列进行ARIMA拟合
for column in imf_data.columns[1:-1]:  # 跳过日期列和残差序列

    # 忽略警告信息
    warnings.filterwarnings("ignore")

    # 使用pmdarima自动寻找基于平滑后的数据的最佳ARIMA参数
    model = pm.auto_arima(imf_data[column],
                          start_p=1, start_q=1,
                          max_p=5, max_q=5,
                          d=None, max_d=2,
                          seasonal=False,
                          trace=False,
                          error_action='ignore',
                          suppress_warnings=True,
                          stepwise=True,
                          n_fits=50)

    # 检查选择的模型是否为(0,0,0)
    print(model.summary())
    print('\n')
    if model.order == (0, 0, 0):
        print(f"Warning: Model for {column} selected (0, 0, 0).")
        model = pm.ARIMA(order=(2, 0, 1)).fit(imf_data[column])

    # 拟合数据
    imf_fitted = model.predict_in_sample()
    imf_fitted_results[column] = imf_fitted

    # 保存模型参数
    params = {
        'IMF': column,
        'p': model.order[0],
        'd': model.order[1],
        'q': model.order[2],
        'AIC': model.aic()
    }
    model_params.append(params)

# 创建一个DataFrame来存储模型参数
# model_params_df = pd.DataFrame(model_params)

# 将拟合结果和模型参数保存到xlsx文件
# with pd.ExcelWriter('IMF_Fitted_Results_and_Models.xlsx') as writer:
#     imf_fitted_results.to_excel(writer, sheet_name='Fitted_Results')
#     model_params_df.to_excel(writer, sheet_name='Model_Parameters')
#
#     # 模型摘要
#     summary_imf = pd.DataFrame(model.summary().tables[1].data[1:], columns=model.summary().tables[1].data[0])
#     summary_filename = f'拟合固有模态函数的模型摘要_{column}.xlsx'
#     summary_imf.to_excel(summary_filename, index=False)

# 保存拟合结果到Excel文件
imf_fitted_results.to_excel('IMF拟合数据.xlsx')

'''子序列线性关系判别'''
linear_subsequences = pd.DataFrame()  # 线性子序列
nonlinear_subsequences = pd.DataFrame()  # 非线性子序列


# 定义距离的计算函数
def distance(c, c_hat):
    return np.mean(np.abs(c - c_hat) / (np.abs(c) + np.abs(c_hat)))


# 计算初始信号的拟合距离
distance_arima = distance(initial_fitted, initial_data.iloc[:, 1])
print('初始信号距离：')
print(distance_arima)

# 创建一个空的列表来保存距离结果
distances = []

# 计算每个IMF的距离
for column in imf_data.columns[1: -1]:  # 跳过日期列
    # 计算距离
    dist = distance(imf_data[column], imf_fitted_results[column])

    # 保存结果
    distances.append({'IMF': column, 'Distance': dist})

    # 分类保存子序列
    if dist >= distance_arima:
        nonlinear_subsequences[column] = imf_data[column]
    else:
        linear_subsequences[column] = imf_data[column]

# 将距离结果转换为DataFrame
distances_df = pd.DataFrame(distances)
print(distances_df)

# 保存距离结果到Excel文件
linear_subsequences.to_excel('线性子序列.xlsx', index=False)
nonlinear_subsequences.to_excel('非线性子序列.xlsx', index=False)

# 合并线性子序列
linear_component = linear_subsequences.sum(axis=1)

'''奇异谱分析'''
trend_component = np.zeros(len(nonlinear_subsequences))
noise_component = np.zeros(len(nonlinear_subsequences))

# 循环处理每一条非线性子序列
for idx, col in enumerate(nonlinear_subsequences.columns):
    series = nonlinear_subsequences[col]
    x = series.index

    # step1 嵌入
    windowLen = windowLen  # 嵌入窗口长度
    seriesLen = len(series)  # 序列长度
    K = seriesLen - windowLen + 1
    X = np.zeros((windowLen, K))
    for i in range(K):
        X[:, i] = series[i:i + windowLen]

    # step2: svd分解， U和sigma已经按升序排序
    U, sigma, VT = np.linalg.svd(X, full_matrices=False)

    for i in range(VT.shape[0]):
        VT[i, :] *= sigma[i]
    A = VT

    # 重组
    rec = np.zeros((windowLen, seriesLen))
    for i in range(windowLen):
        for j in range(windowLen - 1):
            for m in range(j + 1):
                rec[i, j] += A[i, j - m] * U[m, i]
            rec[i, j] /= (j + 1)
        for j in range(windowLen - 1, seriesLen - windowLen + 1):
            for m in range(windowLen):
                rec[i, j] += A[i, j - m] * U[m, i]
            rec[i, j] /= windowLen
        for j in range(seriesLen - windowLen + 1, seriesLen):
            for m in range(j - seriesLen + windowLen, windowLen):
                rec[i, j] += A[i, j - m] * U[m, i]
            rec[i, j] /= (seriesLen - j)

    sigma = rec

    # 提取主趋势层和噪声层
    threshold = threshold_index[idx]  # Access threshold_index using idx
    trend = rec[:threshold, :].sum(axis=0)
    trend_component += trend
    noise = series - trend
    noise_component += noise
    # 保存主趋势层和噪声层
    trend_noise_df = pd.DataFrame({'主趋势层': trend, '噪声层': noise})
    trend_noise_df.to_excel(f'主趋势层和噪声层_{col}.xlsx', index=False)

    # 可视化
    plt.figure(figsize=(16, 5))
    plt.plot(initial_data.index, trend + noise, label='非线性子序列')
    plt.plot(initial_data.index, trend, label='主要趋势层')
    plt.plot(initial_data.index, noise, label='噪声层')
    plt.xlabel('Date', fontsize=14)
    plt.ylabel('Value', fontsize=14)
    plt.legend()
    png_name = f'SSA_{col}.png'
    plt.savefig(png_name, dpi=600)
    plt.show()

# 合并线性分量和主趋势层
linear_component += trend_component

# 保存线性分量和非线性分量
linear_component.to_excel('线性分量.xlsx', index=False)
noise_component = pd.DataFrame(noise_component)
noise_component.to_excel(r'非线性分量.xlsx', index=False)
