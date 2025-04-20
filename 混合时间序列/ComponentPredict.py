import pmdarima as pm
import optuna
import numpy as np
import pandas as pd
from lightgbm import LGBMRegressor
from sklearn.model_selection import train_test_split
from sklearn.model_selection import cross_validate
from sklearn.model_selection import KFold
import matplotlib.pyplot as plt
from matplotlib import rcParams

rcParams['font.family'] = 'sans-serif'
rcParams['font.sans-serif'] = ['Microsoft YaHei']

data = pd.read_excel(r'D:\0海大数模校赛\线性分量和非线性分量.xlsx')
initial_data = pd.read_excel(r'D:\0海大数模校赛\initial_data.xlsx')
initial_data['Date'] = pd.to_datetime(initial_data['Date'], format='%Y年%m月')
data['Year'] = initial_data['Date'].dt.year
data['Month'] = initial_data['Date'].dt.month
data['Day'] = initial_data['Date'].dt.day
linear_component = data.iloc[:, 0]
nonlinear_component = data.iloc[:, 1]
future_predictions = []  # 存储预测结果

'''需要调整的地方'''
n_periods = 10 # 预测期数
training_trial = 100  # 超参数寻优迭代次数

'''绘制初始信号与线性非线性分量的可视化'''
plt.figure(figsize=(16, 5))
plt.plot(initial_data.index, initial_data['Value'], label='初始信号')
plt.plot(initial_data.index, data['线性分量'], label='线性分量')
plt.plot(initial_data.index, data['非线性分量'], label='非线性分量')
plt.xlabel('Date', fontsize=14)
plt.ylabel('Value', fontsize=14)
plt.legend()
plt.savefig(r'数据预处理可视化.png', dpi=600)
plt.show()

'''线性分量预测'''
model = pm.auto_arima(linear_component.dropna(),
                      start_p=1, start_q=1,
                      max_p=5, max_q=5,
                      d=None, max_d=2,
                      seasonal=False,
                      trace=False,
                      error_action='ignore',
                      suppress_warnings=True,
                      stepwise=True,
                      n_fits=50)
print(model.summary())
print('\n')

# 拟合线性分量
# li_fit = model.predict_in_sample()
# li_fit.to_excel(r'线性分量拟合结果.xlsx')

forecast = model.predict(n_periods=n_periods)
print(f"未来{n_periods}期的预测结果:")
print(forecast)
forecast = pd.DataFrame(forecast)
forecast.to_excel(r'线性分量预测结果.xlsx', index=False)

'''非线性分量预测'''
X = data[['Year', 'Month', 'Day', 'PM10', 'O3', 'SO2', 'PM2.5', 'NO2', 'CO']]
y = nonlinear_component


# 定义优化目标函数
def LightGBM_objective(trial):
    n_estimators = trial.suggest_int('n_estimators', 1, 1000, 1)
    max_depth = trial.suggest_int('max_depth', 1, 200, 1)
    learning_rate = trial.suggest_float('learning_rate', 0.001, 0.999, log=True)
    subsample = trial.suggest_float('subsample', 0.05, 0.99)
    min_child_weight = trial.suggest_int('min_child_weight', 1, 300)
    gamma = trial.suggest_float('gamma', 0, 1)
    colsample_bytree = trial.suggest_float('colsample_bytree', 0.1, 1.0)
    reg_alpha = trial.suggest_float('reg_alpha', 0, 1)
    reg_lambda = trial.suggest_float('reg_lambda', 0, 1)

    clf = LGBMRegressor(n_estimators=n_estimators,
                        max_depth=max_depth,
                        learning_rate=learning_rate,
                        subsample=subsample,
                        min_child_weight=min_child_weight,
                        gamma=gamma,
                        colsample_bytree=colsample_bytree,
                        reg_alpha=reg_alpha,
                        reg_lambda=reg_lambda,
                        random_state=1412,
                        verbosity=-1)

    X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    clf.fit(X_train, y_train)

    # 使用划分好的验证集进行交叉验证评估
    cv = KFold(n_splits=10, shuffle=True, random_state=1412)
    validation = cross_validate(clf, X_test, y_test,
                                scoring='neg_mean_absolute_error',
                                cv=cv,
                                verbose=False,
                                n_jobs=-1,
                                error_score='raise')

    return np.mean(validation['test_score'])


# 贝叶斯优化器
def optimizer_optuna(n_trials, algo, objective):
    if algo == 'TPE':
        sampler = optuna.samplers.TPESampler(n_startup_trials=50, n_ei_candidates=100)
    elif algo == 'GP':
        from optuna.integration import SkoptSampler
        import skopt
        sampler = SkoptSampler(skopt_kwargs={'base_estimator': 'GP',
                                             'n_initial_points': 10,
                                             'acq_func': 'EI'})

    study = optuna.create_study(sampler=sampler, direction='maximize')

    scores = []

    for _ in range(n_trials):
        study.optimize(objective, n_trials=1, show_progress_bar=False)
        best_score = study.best_value
        scores.append(best_score)

    best_params = study.best_params

    return best_params, scores


# 使用TPE过程进行贝叶斯优化
LightGBM_best_params, lightGBM_scores = optimizer_optuna(training_trial, 'TPE', LightGBM_objective)
results = {
    'LightGBM': (LightGBM_best_params, lightGBM_scores),
}

for algorithm, (best_params, scores) in results.items():
    print(f'{algorithm} - Best Parameters: {best_params}, Best Scores: {scores}')

results = pd.DataFrame(results)
results.to_excel(r'非线性分量预测超参数组合.xlsx', index=False)

# 使用最佳超参数重新训练模型并进行预测
best_clf = LGBMRegressor(random_state=1412, verbosity=-1, **best_params)
best_clf.fit(X, y)

# 拟合非线性分量
# nor_fit = best_clf.predict(X)
# nor_fit = pd.DataFrame(nor_fit)
# nor_fit.to_excel(r'非线性分量拟合结果.xlsx', index=False)

# 输出未来n天的预测结果
future_prediction = best_clf.predict(X.iloc[-n_periods:])  # 使用模型进行预测
print(f"未来{n_periods}天的预测值:", future_prediction)

future_prediction = pd.DataFrame(future_prediction, columns=['非线性分量预测'])
future_prediction.to_excel(r'非线性分量预测结果.xlsx', index=False)