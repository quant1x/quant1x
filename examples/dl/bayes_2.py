import numpy as np
x = np.random.randint(5, size=(6, 10))
y = np.array([1, 2, 3, 4, 5, 6])
from sklearn.naive_bayes import MultinomialNB
clf = MultinomialNB()
clf.fit(x, y)
print('------- feature vector -------')
print(x)
print('------- prior of each class ------')
print(clf.class_log_prior_)
print('------- probability of features in each class ------')
print(clf.feature_log_prob_)
print('------- predict ------')
print(clf.predict(x[2:3]))
