import matplotlib.pyplot as plt
import numpy as np
from sklearn.linear_model import LinearRegression
from sklearn.preprocessing import PolynomialFeatures


def uniform(size):
    x = np.linspace(0, 1, size)
    return x.reshape(size, 1)


def create_data(size):
    x = uniform(size)
    # x = np.arange(0,size)
    np.random.seed(42)
    y = sin_fun(x) + np.random.normal(scale=0.15, size=x.shape)
    return x, y


def sin_fun(x):
    return np.sin(2 * np.pi * x)


X_train, y_train = create_data(10)

print('X_train =', X_train)
print('y_train =', y_train)

X_test = uniform(100)
y_test = sin_fun(X_test)

plt.scatter(X_train, y_train, facecolor="none", edgecolor="b", s=50, label="training data")
plt.plot(X_test, y_test, c="g", label="$\sin(2\pi x)$")
plt.ylabel("y", size=20)
plt.xlabel("x", size=20)
plt.legend()
plt.show()

fig = plt.figure(figsize=(12, 8))
for i, order in enumerate([0, 1, 3, 9]):
    plt.subplot(2, 2, i + 1)

    poly = PolynomialFeatures(order)
    X_train_ploy = poly.fit_transform(X_train)
    X_test_ploy = poly.fit_transform(X_test)

    lr = LinearRegression()
    lr.fit(X_train_ploy, y_train)
    y_pred = lr.predict(X_test_ploy)

    plt.scatter(X_train, y_train, facecolor="none", edgecolor="b", s=50, label="training data")
    plt.plot(X_test, y_pred, c="r", label="fitting")
    plt.plot(X_test, y_test, c="g", label="$\sin(2\pi x)$")
    plt.title("M={}".format(order))
    plt.legend()
plt.show()
