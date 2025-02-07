import numpy as np
from numpy.random import rand
from functionHO import Fun


def init_position(lb, ub, N, dim):
    """
    初始化位置
    :param lb: 下界
    :param ub: 上界
    :param N: 种群数量
    :param dim: 维度，特征数量
    :return:    位置
    """
    X = np.zeros([N, dim], dtype='float')  # 初始化位置
    for i in range(N):
        for d in range(dim):
            X[i, d] = lb[0, d] + (ub[0, d] - lb[0, d]) * rand()  # 在问题的搜索空间内初始化一组个体的位置

    return X


def binary_conversion(X, thres, N, dim):
    """
    二进制转换
    :param X:   位置
    :param thres:   阈值
    :param N:   种群数量
    :param dim: 维度，特征数量
    :return:    二进制位置，特征是选还是不选
    """
    Xbin = np.zeros([N, dim], dtype='int')  # 初始化二进制位置，这里是直接阈值划分，但可以改进传递函数
    for i in range(N):
        for d in range(dim):
            if X[i, d] > thres:
                Xbin[i, d] = 1
            else:
                Xbin[i, d] = 0

    return Xbin


def roulette_wheel(prob):
    """
    轮盘赌选择,根据适应度值的逆概率来选择个体进行交叉
    :param prob:    每个个体的选择概率
    :return:    选择的索引
    """
    num = len(prob)  # 获取个体数目
    C = np.cumsum(prob)  # 计算累积概率，假设 prob = [0.1, 0.3, 0.6]，则 C = np.cumsum(prob) 会得到 [0.1, 0.4, 1.0]
    P = rand()  # 生成一个随机数
    for i in range(num):  # 遍历所有个体
        if C[i] > P:  # 如果当前累积概率大于随机数，说明选择了第i个个体
            index = i;  # 记录该个体的索引
            break

    return index  # 返回选择的个体索引


def jfs(xtrain, ytrain, opts):
    """
    特征筛选核心代码
    :param xtrain:  训练数据
    :param ytrain:  训练标签
    :param opts:    参数字典
    :return:    特征筛选结果
    """
    # 超参数
    thres = 0.5  # 阈值
    N = opts['N']  # 种群数量
    max_iter = opts['T']  # 最大迭代次数
    CR = opts['CR']  # 交叉概率
    MR = opts['MR']  # 变异概率

    # 特征数量
    dim = np.size(xtrain, 1)
    ub = 1 * np.ones([1, dim], dtype='float')  # 初始化上界，每个dim都初始化为1
    lb = 0 * np.ones([1, dim], dtype='float')  # 初始化下界，每个dim都初始化为0

    # 调用初始化位置函数
    X = init_position(lb, ub, N, dim)

    # 调用二进制转换函数
    Xbin = binary_conversion(X, thres, N, dim)

    # 初始化适应度
    fit = np.zeros([N, 1], dtype='float')  # 每个个体都有一个适应度
    Xgb = np.zeros([1, dim], dtype='int')  # 最佳特征选择向量
    fitG = float('inf')  # 全局最优适应度

    for i in range(N):  # 种群里面一共有N个个体
        fit[i, 0] = Fun(xtrain, ytrain, Xbin[i, :], opts)  # 调用适应度计算函数，计算的是选择的这些特征后输入KNN返回一个适应度值
        if fit[i, 0] < fitG:  # 更新全局最优，当前适应度值和全局最佳适应度值进行对比
            Xgb[0, :] = Xbin[i, :]  # 更新最佳特征选择向量
            fitG = fit[i, 0]  # 更新全局最优适应度

    # 存储收敛曲线
    curve = np.zeros([1, max_iter], dtype='float')
    t = 0

    curve[0, t] = fitG.copy()
    print("Generation:", t + 1)
    print("Best (GA):", curve[0, t])
    t += 1

    # 遗传算法的迭代过程
    while t < max_iter:
        inv_fit = 1 / (1 + fit)  # 适应度越高，适应度的逆越小
        # print('inv_fit:\n', inv_fit)
        prob = inv_fit / np.sum(inv_fit)  # 选择概率，会进行归一化处理,inv_fit和prob都是Nx1的向量
        # print('prob:\n',prob)

        Nc = 0
        for i in range(N):
            if rand() < CR:  # 通过产生一个随机数与交叉概率比较，来决定是否进行交叉
                Nc += 1  # 交叉个体数量
        # print('Nc:\n', Nc)

        # 初始化两个新的后代个体
        x1 = np.zeros([Nc, dim], dtype='int')
        x2 = np.zeros([Nc, dim], dtype='int')
        for i in range(Nc):
            # 通过轮盘赌根据适应度来选择父母个体，传入的是一个个体的选择概率向量，返回的是选择的个体索引
            k1 = roulette_wheel(prob)
            k2 = roulette_wheel(prob)
            P1 = Xbin[k1, :].copy()
            P2 = Xbin[k2, :].copy()

            index = np.random.randint(low=1, high=dim - 1)  # 随机选择交叉点，也就是选择父代个体基因片段的切割点

            # 交叉
            x1[i, :] = np.concatenate((P1[0:index], P2[index:]))  # 第i个父代的孩子1
            x2[i, :] = np.concatenate((P2[0:index], P1[index:]))  # 第i个父代的孩子2

            # 变异
            for d in range(dim):
                if rand() < MR:
                    x1[i, d] = 1 - x1[i, d]

                if rand() < MR:
                    x2[i, d] = 1 - x2[i, d]

        Xnew = np.concatenate((x1, x2), axis=0)
        # print(np.shape(x1))
        # print(np.shape(x2))
        # print(np.shape(Xnew))

        # 交叉变异后的适应度计算
        Fnew = np.zeros([2 * Nc, 1], dtype='float')
        for i in range(2 * Nc):
            Fnew[i, 0] = Fun(xtrain, ytrain, Xnew[i, :], opts)
            if Fnew[i, 0] < fitG:
                Xgb[0, :] = Xnew[i, :]
                fitG = Fnew[i, 0]

        # 存储结果
        curve[0, t] = fitG.copy()
        print("Generation:", t + 1)
        print("Best (GA):", curve[0, t])
        t += 1

        # 精英主义
        XX = np.concatenate((X, Xnew), axis=0)
        FF = np.concatenate((fit, Fnew), axis=0)

        ind = np.argsort(FF, axis=0)  # 对适应度数组FF进行降序，返回的是排序后的索引ind
        # 将当前种群和新生成的后代合并，通过排序选择最优的个体，并将最优个体保留到下一代中
        for i in range(N):
            X[i, :] = XX[ind[i, 0], :]
            fit[i, 0] = FF[ind[i, 0]]

    # 迭代t轮后存储最佳特征选择向量
    Gbin = Xgb[0, :]  # 最佳特征选择向量
    Gbin = Gbin.reshape(dim)  # 转换为一维数组
    pos = np.asarray(range(0, dim))  # 生成一个从0到dim-1的数组
    sel_index = pos[Gbin == 1]  # 选择的特征索引
    num_feat = len(sel_index)  # 选择的特征数量
    ga_data = {'sf': sel_index, 'c': curve, 'nf': num_feat} # 存储结果：特征选择索引，收敛曲线数组，选择的特征数量

    return ga_data
