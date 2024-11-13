## LASSO回归

> 这部分笔记参考[近端梯度(Proximal Gradient)下降算法的过程以及理解|ISTA算法|LASSO问题 - 红泥小火炉的文章 - 知乎](https://zhuanlan.zhihu.com/p/530839064)、[Lasso回归与近端梯度下降推导与实现Posted by Welt Xing on September 30, 2021](https://welts.xyz/2021/09/30/lasso/)、[【西瓜书】 第11章 特征选择与稀疏学习笔记](https://www.jianshu.com/p/0987a8841573)、[利普希茨连续的几何意义是什么？怎么较好的理解它呢？ - cielo的回答 - 知乎](https://www.zhihu.com/question/51809602/answer/151391605)、[L1范数中为什么需要证明目标函数的梯度满足Lipschitz（利普希茨）连续？ - Spectre的回答 - 知乎](https://www.zhihu.com/question/620740222/answer/3531321893)

### L-Lipschitz条件

本文使用近端梯度下降法对LASSO进行求解，近端梯度下降要求如下形式的$\nabla f$满足$Lipschitz$条件
$$
\min_x\ \ f(x)+h(x)\tag{2-1}
$$
因此，本文首先证明LASSO表达式的第一部分的梯度满足$Lipschitz$条件，即证明线性回归模型的均方误差满足$Lipschitz$条件。

#### L-Lipschitz连续的定义

这里引用知乎用户cielo的回答，说得非常形象。

> $Lipschitz$连续，要求函数图像的曲线上任意两点连线的斜率一致有界，就是任意的斜率都小于同一个常数，这个常数就是$Lipschitz$常数。
>
> 从局部看：我们可以取两个充分接近的点，如果这个时候斜率的极限存在的话，这个斜率的极限就是这个点的导数。也就是说函数可导，又是$Lipschitz$连续，那么导数有界。反过来，如果可导函数，导数有界，可以推出函数$Lipschitz$连续。
>
> 从整体看：$Lipschitz$连续要求函数在无限的区间上不能有超过线性的增长，所以$x^2$,$e^x$这些函数在无限区间上不是$Lipschitz$连续的。

这里可以通过维基百科的示意图进行辅助解释：

![img](https://chuantaoli.oss-cn-beijing.aliyuncs.com/v2-4f641af0fc1237b725b7deba376f8aff_1440w.webp)

<center>
    图6 维基百科上对Lipschitz条件的示意图
</center>


白色锥形的斜率就是$Lipschitz$常数，绿色区域的部分则代表满足$Lipschitz$连续，即定义中的“要求函数图像的曲线上任意两点连线的斜率一致有界，就是任意的斜率都小于同一个常数”。

对于定义中的局部，当函数在某点处可导且导数存在时，$Lipschitz$连续性可以通过导数的有界性来保证。如果函数在每一点的导数都是有界的，那么这个函数是$Lipschitz$连续的。导数的有界性保证了函数变化的速率不会超过某个常数。

对于定义中的整体，$x^2$的导数是$2x$，导数随$x$增大而线性增长，因此在无穷远处$x^2$的变化速率没有上界。同理，$e^x$的导数是$e^x$，其变化速率指数增长，所以$e^x$也不是$Lipschitz$连续的。

#### L-Lipschitz条件的证明

定义线性回归的均方误差函数为：
$$
f(w)=\frac{1}{2n}\lVert y-Xw \rVert^2_2\tag{2-2}
$$
LASSO回归的$f(x)$部分与线性回归的均方误差函数相同，下面证明该部分满足Lipschitz条件，定义：
$$
F(x_1,x_2)=\lVert \nabla f(x_1) - \nabla f(x_2) \rVert_2^2 - L\lVert x_1-x_2 \rVert_2^2\tag{2-3}
$$
首先求解$\nabla f(w)$，对式(2-2)进行展开得：
$$
f(w)=\frac{1}{2n}(y-Xw)^T(y-Xw)
$$
根据矩阵求导法则：
$$
\frac{∂}{∂w}(a^Ta)=2a^T\frac{∂a}{∂w}
$$
令$a=(y-Xw)$，对式(2-2)求导后转置，故$\nabla f(w)$为：
$$
\nabla f(w)=-\frac{1}{n}X^T(y-Xw)
$$
代入式(2-3)得：
$$
F(w_1,w_2)=\lVert \frac{1}{n}X^T(Xw_1-y) +\frac{1}{n}X^T(Xw_2+y) \rVert_2^2-L\lVert w_1-w_2 \rVert_2^2\\
=\frac{1}{n^2}\lVert X^TX(w_1-w_2) \rVert_2^2 - L\lVert w_1-w_2 \rVert_2^2\\
$$
现在要证明$F(w_1,x_2)\le0$，则要证明下式：
$$
\frac{\lVert X^TX(w_1-w_2) \rVert_2^2}{n^2\lVert w_1-w_2 \rVert_2^2}\le L
$$
记$A = X^T X$，实际上是证明对于任意的$x$，下式有界：
$$
\frac{\lVert Ax \rVert_2^2}{\lVert x \rVert_2^2}\tag{2-4}
$$
由于$A$是实对称矩阵，因此可以被对角化，即存在一个正交矩阵$Q$和一个对角矩阵$\Lambda$，使得：
$$
A=Q\Lambda Q^T
$$
其中，$Q$是$A$的特征向量，$\Lambda$的对角线元素是$A$的特征值。

对于特征向量的正交性，如果$v_i$和$v_j$对应不同的特征值$（\lambda_i \neq \lambda_j）$，则它们是正交的，即$v_i^T v_j = 0$。除此之外，可以进一步选择特征向量使其单位化，即$\lVert v_i\rVert = 1$，使得它们形成一个标准正交基。

因此，$x$可以表示为$A$的特征向量的线性组合：
$$
x=\sum_{i=1}^n{k_iv_i}
$$
其中，$v_i$是$A$的特征向量，$k_i$是系数。

因此，$Ax$可以化简为：
$$
Ax=A(\sum_{i=1}^n{k_iv_i})=\sum_{i=1}^n{k_iAv_i}=\sum_{i=1}^n{k_i\lambda_iv_i}
$$
$\lVert Ax \rVert_2^2$可以化简为：
$$
\lVert Ax \rVert_2^2\lVert \sum_{i=1}^n{k_i\lambda_iv_i} \rVert_2^2=\sum_{i=1}^n(k_i\lambda_i)^2\lVert v_i\rVert_2^2=\sum_{i=1}^n(k_i\lambda_i)^2
$$
同理，$\lVert x \rVert_2^2$可以化简为：
$$
\lVert x \rVert_2^2=\sum_{i=1}^{n}{k_i}^2
$$
综上所述，可以将式(2-4)化简为：
$$
\frac{\lVert Ax \rVert_2^2}{\lVert x \rVert_2^2}=\frac{\sum_{i=1}^n(k_i\lambda_i)^2}{\sum_{i=1}^{n}{k_i}^2}\tag{2-5}
$$
式(2-5)是一个加权平均式，因此有如下结论：
$$
\min_i{(\lambda_i^2)}\le \frac{\sum_{i=1}^n(k_i\lambda_i)^2}{\sum_{i=1}^{n}{k_i}^2}\ \le\max_i(\lambda_i^2)
$$
因此式(2-4)有界，即：
$$
\frac{\lVert X^TX(w_1-w_2) \rVert_2^2}{\lVert w_1-w_2 \rVert_2^2}\le\max_i(\lambda_i^2)
$$
接下来求解符合条件的$L$：
$$
F(w_1,w_2)=\frac{1}{n^2}\lVert X^TX(w_1-w_2) \rVert_2^2 - L\lVert w_1-w_2 \rVert_2^2\\
=L\lVert w_1-w_2 \rVert_2^2(\frac{\lVert X^TX(w_1-w_2) \rVert_2^2}{n^2L\lVert w_1-w_2 \rVert_2^2}-1)\\
\le L\lVert w_1-w_2 \rVert_2^2(\frac{1}{n^2L}\max_i(\lambda_i^2)-1)
\le 0
$$
故可得$Lipschitz$常数的上界：
$$
L\ge \frac{\max_i(\lambda_i^2)}{n^2}
$$

### 近端梯度下降

#### 近端梯度下降的定义

在上一节已经证明了LASSO的$\nabla f$满足$Lipschitz$条件，因此可以使用近端梯度下降算法进行求解。实际上，近端梯度下降的本质可以视为先对光滑的函数$f(x)$进行梯度下降，再对正则项进行隐性的梯度下降。梯度的$Lipschitz$连续性要求梯度的变化不能过于剧烈，保证了存在合适的步长，使得$f(x)$沿着梯度方向前进这个步长可以使函数值下降（即descent lemma），这可用于证明算法收敛性。

近端梯度算法（Proximal Gradient Descent, PGD）常用来解决如下形式的优化问题：
$$
\min_x\ \ f(x)+h(x) \tag{2-6}
$$
近端梯度下降的$h(x)$有以下几种形式：

1. $h(x)=0$：则为一般的梯度下降算法。

2. $h(x)=I_C(x)$：则称为投影梯度下降（Projected gradient descent），其中：
   $$
   I_C(x)=\begin{cases}0,&x\in C\\
   \infty,&x\notin C\end{cases}
   $$

3. $h(x)=\lambda\lVert x\rVert_1$：则被称为迭代收缩阈值算法（Iterative Shrinkage Thresholding Algorithm,ISTA），LASSO回归为该种类型。

#### 迭代方法

对于函数$f(x)$，在$x_k$附近进行二阶泰勒展开得：
$$
\hat f(x)\simeq f(x_k)+\nabla f(x_k)^T(x-x_k)+\frac{1}{2}(x-x_k)^T\nabla ^2f(x_k)(x-x_k)
$$
上式中，$\nabla ^2f(x_k)$为$Hesse$矩阵，并且正定，$Lipschitz$常数是$Hesse$矩阵的上界，在优化算法中常这样进行简化，故上式可以化简为如下形式：
$$
\hat f(x)=f(x_k)+\lang \nabla f(x_k),(x-x_k)\rang+\frac{L}{2}\lVert (x-x_k)\rVert _2^2
$$
进一步化简：
$$
\hat f(x)=f(x_k)+\lang \nabla f(x_k),(x-x_k)\rang+\lang \frac{L}{2}(x-x_k),x-x_k \rang\\
=f(x_k)+\lang \nabla f(x_k)+\frac{L}{2}(x-x_k),x-x_k \rang\\
=f(x_k)+\frac{L}{2}\lang\frac{L}{2} \nabla f(x_k)+(x-x_k),x-x_k \rang\\
=f(x_k)+\frac{L}{2}\lang x-x_k+\frac{1}{L}\nabla f(x_k)+ \frac{1}{L}\nabla f(x_k),x-x_k+\frac{1}{L}\nabla f(x_k)-\frac{1}{L}\nabla f(x_k)\rang\\
=f(x_k)+\frac{L}{2}\lVert x-x_k+\frac{1}{L}\nabla f(x_k)\rVert_2^2-\frac{1}{2L}\lVert \nabla f(x_k) \rVert_2^2\\
=\frac{L}{2}\lVert x-(x_k-\frac{1}{L}\nabla f(x_k)) \rVert_2^2+const
$$
显然，上式的最小值在如下$x_{k+1}$获得：
$$
x_{k+1}=x_k-\frac{1}{L}\nabla f(x_k)
$$
下面证明通过迭代上式可以使$f(x)$逐步下降。

#### Descent Lemma的证明

对于函数$f(x)$，由定积分的定义，则：
$$
f(x_2)-f(x_1)=\int_{x_1}^{x_2}f'(x)dx
$$
设$f(x)$二次连续可微，$x_k\in\R^n$，令$g(t)=f(x_k+t(x-x_k))$，则：
$$
g(0)=f(x_k)
\\g(1)=f(x)
$$
因此，由定积分的定义可得：
$$
f(x)-f(x_k)=g(1)-g(0)=\int_0^1{g'(t)dt}\tag{2-7}
$$
而对于$g'(t)$，有：
$$
g'(t)=\frac{\part f(x_k+t(x-x_k))}{\part t}\\
=\nabla f(x_k+t(x-x_k))^T(x-x_k)
$$
代入式(2-7)可得：
$$
f(x)-f(x_k)=\int_0^1{\nabla f(x_k+t(x-x_k))^T(x-x_k)}dt\\
=\int_0^1[\nabla f(x_k+t(x-x_k))-\nabla f(x_k)+\nabla f(x_k)]^T(x-x_k)dt\\
=\int_0^1\nabla f(x_k)^T(x-x_k)dt+\int_0^1[\nabla f(x_k+t(x-x_k))-\nabla f(x_k)]^T(x-x_k)dt\\ \tag{2-8}
$$
对上式使用如下内积性质进行放缩：
$$
x^Ty=\lVert x \rVert \cdot \lVert y \rVert \cos\theta\le \lVert x \rVert \cdot \lVert y \rVert
$$
故式(2-8)可以放缩为如下形式：
$$
f(x)-f(x_k)\le\int_0^1\nabla f(x_k)^T(x-x_k)dt+\int_0^1\lVert\nabla f(x_k+t(x-x_k))-\nabla f(x_k)\rVert_2\cdot \lVert x-x_k\rVert _2dt\\
$$
接下来对上式使用$Lipschitz$常数进行放缩，可得到：
$$
f(x)-f(x_k)\le\int_0^1\nabla f(x_k)^T(x-x_k)dt+\lVert x-x_k\rVert _2\int_0^1L\lVert t(x-x_k)\rVert _2dt
$$
对上式进行定积分，可化简为：
$$
f(x)-f(x_k)=\nabla f(x_k)^T(x-x_k)+\frac{L}{2}\lVert (x-x_k)\rVert _2^2
$$
令
$$
\hat f(x)=f(x_k)+\nabla f(x_k)^T(x-x_k)+\frac{L}{2}\lVert (x-x_k)\rVert _2^2\tag{2-9}
$$
则有$f(x)\le\hat f(x)$，当$x=x_k$时等号成立，该不等式被称为descent lemma。

由于$f(x)$在$x=x_k-\frac{1}{L}\nabla f(x_k)$处取得最小值，因此以下不等式成立：
$$
\hat f(x_k-\frac{1}{L}\nabla f(x_k))\le \hat f(x_k)
$$
根据descent lemma的推导有$f(x)\le\hat f(x)$恒成立，因此以下不等式也成立：
$$
f(x_k-\frac{1}{L}\nabla f(x_k))\le \hat f(x_k-\frac{1}{L}\nabla f(x_k))
$$
且当$x=x_k$时，由式(2-9)可知$f(x_k)=\hat f(x_k)$，因此：
$$
f(x_k-\frac{1}{L}\nabla f(x_k))\le \hat f(x_k-\frac{1}{L}\nabla f(x_k))\le\hat f(x_k)=f(x_k)
$$
综上所述，可以通过迭代下式使得$f(x)$逐步下降：
$$
x_{k+1}=x_k-\frac{1}{L}\nabla f(x_k)\tag{2-10}
$$

#### 求解LASSO

上面的证明和推导都是针对LASSO的第一部分，LASSO相对于线性回归只是加上了$L1$范数，故将式(2-10)应用在LASSO回归上，可得到如下迭代式：
$$
x_{k+1}=\underset{x}{\arg\min}\ \frac{L}{2}\lVert x-(x_k-\frac{1}{L}\nabla f(x_k)) \rVert_2^2+\lambda\lVert x\rVert_1\\
$$
令$z=x_k-\frac{1}{L}\nabla f(x_k)$，$L=\frac{1}{t}$其实际上为$f(x)$的梯度下降形式，可以将上式转换成如下形式：
$$
x_{k+1}=\underset{x}{\arg\min}\ \frac{1}{2}\lVert x-z \rVert_2^2+\lambda t\lVert x\rVert_1\tag{2-11}
$$
式(2-11)也被称为近端算子（proximal operator），记为：
$$
prox_{t,h(\cdot)}(z)=\underset{x}{\arg\min}\ \frac{1}{2}\lVert x-z\rVert_2^2+t\cdot h(x)
$$
因此，对于式(2-1)的问题，近端梯度下降算法的一般迭代过程是先对$f(x)$进行梯度下降求得$z^{(k)}$，然后代入近端算子$prox_{t,h(\cdot)}(z^{(k)})$，然后迭代求解：
$$
z^{(k)}=x^{(k)}-t\nabla f(x^{(k)})\\
x^{(k+1)}=prox_{t,h(\cdot)}(z^{(k)})
$$

#### 总结

LASSO回归由于其L1正则化的稀疏性，因此成为一种可以降低多重共线性的特征筛选方法。然而，L1正则化只能针对单个特征进行考虑，故没有筛选到的特征并不代表不重要，在遇到两个高度相关特征的时候，难以确定要保留哪个。在2020年，Gao Wang等人在《Journal of the Royal Statistical Society Series B: Statistical Methodology》发表了一篇名为《A Simple New Approach to Variable Selection in Regression, with Application to Genetic Fine Mapping》的论文，提出了SuSIE这一能够考虑多个特征交互影响的方特征筛选法和逐步迭代求解SuSIE的IBSS算法。