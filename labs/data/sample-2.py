#!/usr/bin/python
# -*- coding: UTF-8 -*-

import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as stats

np.random.seed(8)


def fit_plot_line(x=[], y=[], ci=95):
    alpha = 1 - ci / 100
    n = len(x)

    Sxx = np.sum(x ** 2) - np.sum(x) ** 2 / n
    Sxy = np.sum(x * y) - np.sum(x) * np.sum(y) / n
    mean_x = np.mean(x)
    mean_y = np.mean(y)

    # Linefit
    b = Sxy / Sxx
    a = mean_y - b * mean_x

    # Residuals
    def fit(xx):
        return a + b * xx

    residuals = y - fit(x)

    var_res = np.sum(residuals ** 2) / (n - 2)
    sd_res = np.sqrt(var_res)

    # Confidence intervals
    se_b = sd_res / np.sqrt(Sxx)
    se_a = sd_res * np.sqrt(np.sum(x ** 2) / (n * Sxx))

    df = n - 2  # degrees of freedom
    tval = stats.t.isf(alpha / 2., df)  # appropriate t value

    ci_a = a + tval * se_a * np.array([-1, 1])
    ci_b = b + tval * se_b * np.array([-1, 1])

    # create series of new test x-values to predict for
    npts = 100
    px = np.linspace(np.min(x), np.max(x), num=npts)

    def se_fit(x):
        return sd_res * np.sqrt(1. / n + (x - mean_x) ** 2 / Sxx)

    # Plot the data
    plt.figure()

    plt.plot(px, fit(px), 'k', label='Regression line')
    plt.plot(x, y, 'k.')

    x.sort()
    limit = (1 - alpha) * 100
    plt.plot(x, fit(x) + tval * se_fit(x), 'r--', lw=2,
             label='Confidence limit ({0:.1f}%)'.format(limit))
    plt.plot(x, fit(x) - tval * se_fit(x), 'r--', lw=2)

    plt.xlabel('X values')
    plt.ylabel('Y values')
    plt.title('Linear regression and confidence limits')
    plt.legend(loc='best')
    plt.show()


# generate data
mean, cov = [4, 6], [(1.5, .7), (.7, 1)]
x, y = np.random.multivariate_normal(mean, cov, 80).T

# fit line and plot figure
fit_plot_line(x=x, y=y, ci=95)
