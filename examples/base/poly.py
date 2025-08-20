from numpy import poly1d, vander, finfo, dot
# 线性回归
import numpy as np
import numpy.core.numeric as NX
from numpy.linalg import lstsq

print('------------------------------------------------------------')
x = [0.0,0.1,0.2,0.3,0.5,0.8,1.0]
y = [1.0,0.41, 0.50, 0.61, 0.91, 2.02, 2.46]
A1 = np.polyfit(x, y, 2)


def polyfit2(x, y, deg, rcond=None, full=False, w=None, cov=False):
    order = int(deg) + 1
    x = NX.asarray(x) + 0.0
    y = NX.asarray(y) + 0.0

    # check arguments.
    if deg < 0:
        raise ValueError("expected deg >= 0")
    if x.ndim != 1:
        raise TypeError("expected 1D vector for x")
    if x.size == 0:
        raise TypeError("expected non-empty vector for x")
    if y.ndim < 1 or y.ndim > 2:
        raise TypeError("expected 1D or 2D array for y")
    if x.shape[0] != y.shape[0]:
        raise TypeError("expected x and y to have same length")

    # set rcond
    if rcond is None:
        rcond = len(x) * finfo(x.dtype).eps

    # set up least squares equation for powers of x
    lhs = vander(x, order)
    print(lhs)
    rhs = y

    # apply weighting
    if w is not None:
        w = NX.asarray(w) + 0.0
        if w.ndim != 1:
            raise TypeError("expected a 1-d array for weights")
        if w.shape[0] != y.shape[0]:
            raise TypeError("expected w and y to have the same length")
        lhs *= w[:, NX.newaxis]
        if rhs.ndim == 2:
            rhs *= w[:, NX.newaxis]
        else:
            rhs *= w

    # scale lhs to improve condition number and solve
    scale = NX.sqrt((lhs * lhs).sum(axis=0))
    lhs /= scale
    c, resids, rank, s = lstsq(lhs, rhs, rcond)
    c = (c.T / scale).T  # broadcast scale coefficients

    # warn on rank reduction, which indicates an ill conditioned matrix
    if rank != order and not full:
        msg = "Polyfit may be poorly conditioned"
        #warnings.warn(msg, RankWarning, stacklevel=4)

    if full:
        return c, resids, rank, s, rcond
    elif cov:
        Vbase = np.inv(dot(lhs.T, lhs))
        Vbase /= NX.outer(scale, scale)
        if cov == "unscaled":
            fac = 1
        else:
            if len(x) <= order:
                raise ValueError("the number of data points must exceed order "
                                 "to scale the covariance matrix")
            # note, this used to be: fac = resids / (len(x) - order - 2.0)
            # it was deciced that the "- 2" (originally justified by "Bayesian
            # uncertainty analysis") is not what the user expects
            # (see gh-11196 and gh-11197)
            fac = resids / (len(x) - order)
        if y.ndim == 1:
            return c, Vbase * fac
        else:
            return c, Vbase[:, :, NX.newaxis] * fac
    else:
        return c


A = polyfit2(x, y, 2)
print(A)

#
# print('------------------------------------------------------------')
# z = np.polyval(A, x)
# print(z)
# print('------------------------------------------------------------')
# p = NX.asarray(A)
# print(p)
# if isinstance(x, poly1d):
#     y = 0
# else:
#     x = NX.asanyarray(x)
#     y = NX.zeros_like(x)
# for pv in p:
#     y = y * x + pv
# print(y)


