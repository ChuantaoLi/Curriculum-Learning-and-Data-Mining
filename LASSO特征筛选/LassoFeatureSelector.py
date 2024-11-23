import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from sklearn.model_selection import train_test_split
from matplotlib import rcParams
from sklearn.exceptions import ConvergenceWarning
from sklearn.linear_model import Lasso, LassoCV
from sklearn.metrics import mean_squared_error
from statsmodels.stats.outliers_influence import variance_inflation_factor
import warnings
import os

pd.set_option('display.max_columns', None)
pd.set_option('display.max_rows', None)

# 设置matplotlib绘图的中文字体
rcParams['font.sans-serif'] = ['Microsoft YaHei']
rcParams['axes.unicode_minus'] = False

# 忽略特定类型的警告
warnings.filterwarnings("ignore", category=ConvergenceWarning)


class LassoFeatureSelector:
    def __init__(self, input_train_data_path, input_val_data_path, output_train_path, output_val_path, input_test_data_path,
                 output_test_path, upper_limit_alpha, lower_limit_alpha, iterations, cv):
        self.input_train_data_path = input_train_data_path
        self.input_val_data_path = input_val_data_path
        self.input_test_data_path = input_test_data_path
        self.output_train_path = output_train_path
        self.output_val_path = output_val_path
        self.output_test_path = output_test_path
        self.upper_limit_alpha = upper_limit_alpha
        self.lower_limit_alpha = lower_limit_alpha
        self.iterations = iterations
        self.cv = cv

    def load_data(self):  # 读取文件函数
        if os.path.splitext(self.input_train_data_path)[1] == '.xlsx':
            self.input_train_data = pd.read_excel(self.input_train_data_path)
        elif os.path.splitext(self.input_train_data_path)[1] == '.csv':
            self.input_train_data = pd.read_csv(self.input_train_data_path)

        if os.path.splitext(self.input_val_data_path)[1] == '.xlsx':
            self.input_val_data = pd.read_excel(self.input_val_data_path)
        elif os.path.splitext(self.input_val_data_path)[1] == '.csv':
            self.input_val_data = pd.read_csv(self.input_val_data_path)

        if os.path.splitext(self.input_test_data_path)[1] == '.xlsx':
            self.input_test_data = pd.read_excel(self.input_test_data_path)
        elif os.path.splitext(self.input_test_data_path)[1] == '.csv':
            self.input_test_data = pd.read_csv(self.input_test_data_path)

    def lasso_regression(self, train, test, alpha):  # LASSO回归函数
        lassoreg = Lasso(alpha=alpha, max_iter=self.iterations, fit_intercept=True, precompute=False,
                         copy_X=False, tol=0.0001, warm_start=False, positive=False, selection='cyclic')
        lassoreg.fit(train.iloc[:, 0:-1], train.iloc[:, -1])
        feature_count = np.sum(lassoreg.coef_ != 0)
        y_pred = lassoreg.predict(test.iloc[:, 0:-1])
        mse = mean_squared_error(test.iloc[:, -1], y_pred)
        ret = [alpha, mse]
        ret.append(feature_count)
        ret.extend(lassoreg.coef_)
        return ret

    def matrix_lasso(self):  # LASSO系数矩阵函数
        self.alpha_lasso = np.linspace(self.lower_limit_alpha, self.upper_limit_alpha, self.iterations)
        col = ["alpha", "mse", "feature_count"] + list(self.input_train_data.columns[:-1])
        ind = ["alpha_%.4g" % self.alpha_lasso[i] for i in range(len(self.alpha_lasso))]
        self.coef_matrix_lasso = pd.DataFrame(index=ind, columns=col)

        input_train_1, input_train_2 = train_test_split(self.input_train_data, test_size=0.2, random_state=42)  # 在输入的训练集里面分割进行LASSO回归
        for i in range(len(self.alpha_lasso)):
            self.coef_matrix_lasso.iloc[i] = self.lasso_regression(input_train_1, input_train_2, self.alpha_lasso[i])
        self.coef_matrix_lasso.to_excel(r'LASSO系数矩阵.xlsx', index=True)

    def plot_lasso_path(self):  # 绘制岭迹图函数
        plt.figure(figsize=(14, 6.8))
        for i in range(len(list(self.input_train_data.columns[:-1]))):
            plt.plot(self.coef_matrix_lasso["alpha"],
                     self.coef_matrix_lasso[list(self.input_train_data.columns[:-1])[i]],
                     color=plt.cm.Set1(i / len(list(self.input_train_data.columns[:-1]))),
                     label=list(self.input_train_data.columns[:-1])[i])
        plt.legend(loc="upper right", ncol=7, prop={'size': 7})
        plt.xlabel("正则化系数", fontsize=14)
        plt.ylabel("回归系数", fontsize=14)
        plt.savefig(r'LASSO回归岭迹图', dpi=600)
        plt.show()

    def select_best_alpha(self):  # 选择最佳正则化系数函数
        alpha_choose = np.linspace(self.lower_limit_alpha, self.upper_limit_alpha, self.iterations)
        lasso_cv = LassoCV(alphas=alpha_choose, cv=self.cv, max_iter=self.iterations)
        lasso_cv.fit(self.input_train_data.iloc[:, :-1], self.input_train_data.iloc[:, -1])
        self.lasso_best_alpha = lasso_cv.alpha_
        print(f"选择的最佳正则化系数: {self.lasso_best_alpha}")

    def calculate_vif(self, data):  # 计算方差膨胀因子函数
        vif = pd.DataFrame()
        vif['特征'] = data.columns
        vif['方差膨胀因子'] = [variance_inflation_factor(data.values, i) for i in range(data.shape[1])]
        return vif

    def fit_lasso_model(self):  # 筛选特征函数
        self.lasso_model = Lasso(alpha=self.lasso_best_alpha, fit_intercept=True,
                                 max_iter=self.iterations, random_state=42, selection='cyclic')
        self.lasso_model.fit(self.input_train_data.iloc[:, :-1], self.input_train_data.iloc[:, -1])
        self.selected_features = self.input_train_data.iloc[:, :-1].columns[self.lasso_model.coef_ != 0]

    def save_vif(self):  # 输出并保存方差膨胀因子函数
        vif_before = self.calculate_vif(self.input_train_data.iloc[:, :-1])
        print("原始数据的方差膨胀因子:\n", vif_before)
        input_data_selected = self.input_train_data.iloc[:, :-1][self.selected_features]
        vif_after = self.calculate_vif(input_data_selected)
        print("筛选特征后的方差膨胀因子:\n", vif_after)

        vif_before.to_excel(r'原始训练集的方差膨胀因子.xlsx', index=False)
        vif_after.to_excel(r'LASSO回归后训练集的方差膨胀因子.xlsx', index=False)

    def save_selected_data(self):  # 导出LASSO筛选特征后的训练集和测试集
        selected_data_train = self.input_train_data[list(self.selected_features) + [self.input_train_data.columns[-1]]]
        selected_data_val = self.input_val_data[list(self.selected_features) + [self.input_val_data.columns[-1]]]
        selected_data_test = self.input_test_data[list(self.selected_features) + [self.input_test_data.columns[-1]]]

        if os.path.splitext(self.output_train_path)[1] == '.xlsx':
            selected_data_train.to_excel(self.output_train_path, index=False)
        elif os.path.splitext(self.output_train_path)[1] == '.csv':
            selected_data_train.to_csv(self.output_train_path, index=False)

        if os.path.splitext(self.output_val_path)[1] == '.xlsx':
            selected_data_val.to_excel(self.output_val_path, index=False)
        elif os.path.splitext(self.output_val_path)[1] == '.csv':
            selected_data_val.to_csv(self.output_val_path, index=False)

        if os.path.splitext(self.output_test_path)[1] == '.xlsx':
            selected_data_test.to_excel(self.output_test_path, index=False)
        elif os.path.splitext(self.output_test_path)[1] == '.csv':
            selected_data_test.to_csv(self.output_test_path, index=False)

    def calculate_avg_vif(self):  # 计算平均方差膨胀因子函数
        vif_before = self.calculate_vif(self.input_train_data.iloc[:, :-1])
        avg_vif_before = vif_before['方差膨胀因子'].mean()
        input_data_selected = self.input_train_data.iloc[:, :-1][self.selected_features]
        vif_after = self.calculate_vif(input_data_selected)
        avg_vif_after = vif_after['方差膨胀因子'].mean()
        print(f"特征筛选前的平均方差膨胀因子: {avg_vif_before}")
        print(f"特征筛选后的平均方差膨胀因子: {avg_vif_after}")

    def run_all(self):
        self.load_data()
        self.matrix_lasso()
        self.plot_lasso_path()
        self.select_best_alpha()
        self.fit_lasso_model()
        self.save_vif()
        self.save_selected_data()
        self.calculate_avg_vif()
