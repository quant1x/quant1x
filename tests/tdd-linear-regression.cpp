#include <quant1x/test/test.h>


using namespace std;

// 定义线性回归类
class LinearRegression {
private:
    double w, b;            // 权重和偏置
    double learning_rate;   // 学习率
    int iterations;         // 迭代次数

public:
    explicit LinearRegression(double lr = 0.01, int iters = 1000)
        : w(0.0), b(0.0), learning_rate(lr), iterations(iters) {}

    // 前向传播, 预测 y
    double predict(double x) const {
        return w * x + b;
    }

    // 损失函数(均方误差)
    double compute_loss(const vector<double>& X, const vector<double>& Y) const {
        double loss = 0.0;
        for (size_t i = 0; i < X.size(); ++i) {
            double y_pred = predict(X[i]);
            loss += pow(y_pred - Y[i], 2);
        }
        return loss / X.size();
    }

    // 梯度下降训练
    void fit(const vector<double>& X, const vector<double>& Y) {
        int n = int(X.size());
        for (int iter = 0; iter < iterations; ++iter) {
            double dw = 0.0, db = 0.0;

            for (int i = 0; i < n; ++i) {
                double y_pred = predict(X[i]);
                dw += (y_pred - Y[i]) * X[i];
                db += (y_pred - Y[i]);
            }

            dw /= n;
            db /= n;

            // 更新参数
            w -= learning_rate * dw;
            b -= learning_rate * db;

            if (iter % 100 == 0) {
                double loss = compute_loss(X, Y);
                cout << "Iter " << iter << ": Loss = " << loss << ", w = " << w << ", b = " << b << endl;
            }
        }
    }

    // 获取参数
    [[nodiscard]] pair<double, double> get_params() const {
        return {w, b};
    }
};

TEST_CASE("v1", "[linear-regression]") {
    // 示例数据: y ≈ 2x + 1
    vector<double> X = {1, 2, 3, 4, 5};
    vector<double> Y = {3, 5, 7, 9, 11};

    LinearRegression model(0.01, 1000);
    model.fit(X, Y);

    auto [w, b] = model.get_params();
    cout << "训练完成, 模型参数: " << endl;
    cout << "w = " << w << ", b = " << b << endl;

    // 测试预测
    double x_test = 6;
    cout << "预测值(x=" << x_test << "): " << model.predict(x_test) << endl;
}