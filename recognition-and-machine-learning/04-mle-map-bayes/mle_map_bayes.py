import numpy as np
from scipy.stats import norm
from scipy.special import gamma
import scipy.special as spec  # for gamma
# importing bayes doesn't work in BRUTE :(, please copy the functions into this file


# MLE
def ml_estim_normal(x):
    """
    Computes maximum likelihood estimate of mean and variance of a normal distribution.

    :param x:   measurements, numpy array (n, )
    :return:    mu - mean - python float
                var - variance - python float
    """
    mu = np.mean(x)
    var = np.mean(np.square(x - mu))
    return float(mu), float(var)


def ml_estim_categorical(counts):
    """
    Computes maximum likelihood estimate of categorical distribution parameters.

    :param counts: measured bin counts, numpy array (n, )
    :return:       pk - parameters of the categorical distribution, numpy array (n, )
    """
    return counts / np.sum(counts)

# MAP
def map_estim_normal(x, mu0, nu, alpha, beta):
    """
    Maximum a posteriori parameter estimation of normal distribution with normal inverse gamma prior.

    :param x:      measurements, numpy array (n, )
    :param mu0:    NIG parameter - python float
    :param nu:     NIG parameter - python float
    :param alpha:  NIG parameter - python float
    :param beta:   NIG parameter - python float

    :return:       mu - estimated mean - python float,
                   var - estimated variance - python float
    """
    mu = (nu * mu0 + np.sum(x)) / (len(x) + nu)
    var = (2 * beta + nu * (mu - mu0) ** 2 + np.sum(np.square(x - mu))) / (len(x) + 3 + 2 * alpha)
    return float(mu), float(var)


def map_estim_categorical(counts, alpha):
    """
    Maximum a posteriori parameter estimation of categorical distribution with Dirichlet prior.

    :param counts:  measured bin counts, numpy array (n, )
    :param alpha:   Dirichlet distribution parameters, numpy array (n, )

    :return:        pk - estimated categorical distribution parameters, numpy array (n, )
    """
    return (counts + alpha - 1) / np.sum(counts + alpha - 1)

# BAYES
def bayes_posterior_params_normal(x, prior_mu0, prior_nu, prior_alpha, prior_beta):
    """
    Compute a posteriori normal inverse gamma parameters from data and NIG prior.

    :param x:            measurements, numpy array (n, )
    :param prior_mu0:    NIG parameter - python float
    :param prior_nu:     NIG parameter - python float
    :param prior_alpha:  NIG parameter - python float
    :param prior_beta:   NIG parameter - python float

    :return:             mu0:    a posteriori NIG parameter - python float
    :return:             nu:     a posteriori NIG parameter - python float
    :return:             alpha:  a posteriori NIG parameter - python float
    :return:             beta:   a posteriori NIG parameter - python float
    """
    N = len(x)
    x_sum = np.sum(x)
    mu0 = (prior_nu * prior_mu0 + x_sum) / (prior_nu + N)
    nu = prior_nu + N
    alpha = prior_alpha + N / 2
    beta = prior_beta + np.sum(np.square(x)) / 2 + (prior_nu * prior_mu0 ** 2) / 2 - \
           np.square(prior_nu * prior_mu0 + x_sum) / (2 * (prior_nu + N))
    return float(mu0), float(nu), float(alpha), float(beta)

def bayes_posterior_params_categorical(counts, alphas):
    """
    Compute a posteriori Dirichlet parameters from data and Dirichlet prior.

    :param counts:   measured bin counts, numpy array (n, )
    :param alphas:   prior Dirichlet distribution parameters, numpy array (n, )

    :return:         posterior_alphas - estimated Dirichlet distribution parameters, numpy array (n, )
    """
    return counts + alphas

def bayes_estim_pdf_normal(x_test, x, mu0, nu, alpha, beta):
    """
    Compute pdf of predictive distribution for Bayesian estimate for normal distribution with normal inverse gamma prior.

    :param x_test:  values where the pdf should be evaluated, numpy array (m, )
    :param x:       'training' measurements, numpy array (n, )
    :param mu0:     prior NIG parameter - python float
    :param nu:      prior NIG parameter - python float
    :param alpha:   prior NIG parameter - python float
    :param beta:    prior NIG parameter - python float

    :return:        pdf - Bayesian estimate pdf evaluated at x_test, numpy array (m, )
    """
    mu0, nu, alpha, beta = bayes_posterior_params_normal(x, mu0, nu, alpha, beta)

    pdf = np.zeros(len(x_test))
    for i, x_star in enumerate(x_test):
        x_star = np.array([x_star])
        mu0_, nu_, alpha_, beta_ = bayes_posterior_params_normal(x_star, mu0, nu, alpha, beta)
        pdf1 = 1 / np.sqrt(2 * np.pi)
        pdf2 = (np.sqrt(nu) * np.power(beta, alpha)) / (np.sqrt(nu_) * np.power(beta_, alpha_))
        pdf3 = gamma(alpha_) / gamma(alpha)
        pdf[i] = pdf1 * pdf2 * pdf3

    return pdf

def bayes_estim_categorical(counts, alphas):
    """
    Compute parameters of Bayesian estimate for categorical distribution with Dirichlet prior.

    :param counts:  measured bin counts, numpy array (n, )
    :param alphas:  prior Dirichlet distribution parameters, numpy array (n, )

    :return:        pk - estimated categorical distribution parameters, numpy array (n, )
    """
    return (counts + alphas) / np.sum(counts + alphas)

# Classification
def mle_Bayes_classif(x_test, x_train_A, x_train_C):
    """
    Classify images using Bayes classification using MLE of normal distributions and 0-1 loss.

    :param x_test:         test image features, numpy array (N, )
    :param x_train_A:      training image features A, numpy array (nA, )
    :param x_train_C:      training image features C, numpy array (nC, )

    :return:               q - classification strategy (see find_strategy_2normal)
    :return:               labels - classification of test_data, numpy array (N, ) (see bayes.classify_2normal)
    :return:               DA - parameters of the normal distribution of A
                            DA['Mean'] - python float
                            DA['Sigma'] - python float
                            DA['Prior'] - python float
    :return:               DC - parameters of the normal distribution of C
                            DC['Mean'] - python float
                            DC['Sigma'] - python float
                            DC['Prior'] - python float
    """
    DA = dict()
    DC = dict()

    DA['Mean'], DA['Sigma'] = ml_estim_normal(x_train_A)
    DA['Sigma'] = float(np.sqrt(DA['Sigma']))
    DA['Prior'] = len(x_train_A) / (len(x_train_A) + len(x_train_C))
    DC['Mean'], DC['Sigma'] = ml_estim_normal(x_train_C)
    DC['Sigma'] = float(np.sqrt(DC['Sigma']))
    DC['Prior'] = len(x_train_C) / (len(x_train_A) + len(x_train_C))

    q = find_strategy_2normal(DA, DC)
    labels = classify_2normal(x_test, q)

    return q, labels, DA, DC


def map_Bayes_classif(x_test, x_train_A, x_train_C,
                      mu0_A, nu_A, alpha_A, beta_A,
                      mu0_C, nu_C, alpha_C, beta_C):
    """
    Classify images using Bayes classification using MAP estimate of normal distributions with NIG priors and 0-1 loss.

    :param x_test:         test image features, numpy array (N, )
    :param x_train_A:      training image features A, numpy array (nA, )
    :param x_train_C:      training image features C, numpy array (nC, )

    :param mu0_A:          prior NIG parameter for A - python float
    :param nu_A:           prior NIG parameter for A - python float
    :param alpha_A:        prior NIG parameter for A - python float
    :param beta_A:         prior NIG parameter for A - python float

    :param mu0_C:          prior NIG parameter for C - python float
    :param nu_C:           prior NIG parameter for C - python float
    :param alpha_C:        prior NIG parameter for C - python float
    :param beta_C:         prior NIG parameter for C - python float

    :return:               q - classification strategy (see find_strategy_2normal)
    :return:               labels - classification of test_imgs, numpy array (N, ) (see bayes.classify_2normal)
    :return:               DA - parameters of the normal distribution of A
                            DA['Mean'] - python float
                            DA['Sigma'] - python float
                            DA['Prior'] - python float
    :return:               DC - parameters of the normal distribution of C
                            DC['Mean'] - python float
                            DC['Sigma'] - python float
                            DC['Prior'] - python float
    """
    DA = dict()
    DC = dict()

    DA['Mean'], DA['Sigma'] = map_estim_normal(x_train_A, mu0_A, nu_A, alpha_A, beta_A)
    DA['Sigma'] = float(np.sqrt(DA['Sigma']))
    DA['Prior'] = len(x_train_A) / (len(x_train_A) + len(x_train_C))
    DC['Mean'], DC['Sigma'] = map_estim_normal(x_train_C, mu0_C, nu_C, alpha_C, beta_C)
    DC['Sigma'] = float(np.sqrt(DC['Sigma']))
    DC['Prior'] = len(x_train_C) / (len(x_train_A) + len(x_train_C))

    q = find_strategy_2normal(DA, DC)
    labels = classify_2normal(x_test, q)

    return q, labels, DA, DC


def bayes_Bayes_classif(x_test, x_train_A, x_train_C,
                        mu0_A, nu_A, alpha_A, beta_A,
                        mu0_C, nu_C, alpha_C, beta_C):
    """
    Classify images using Bayes classification (0-1 loss) using predictive pdf estimated using Bayesian inferece with with NIG priors.

    :param x_test:         images features to be classified, numpy array (n, )
    :param x_train_A:      training image features A, numpy array (nA, )
    :param x_train_C:      training image features C, numpy array (nC, )

    :param mu0_A:          prior NIG parameter for A - python float
    :param nu_A:           prior NIG parameter for A - python float
    :param alpha_A:        prior NIG parameter for A - python float
    :param beta_A:         prior NIG parameter for A - python float

    :param mu0_C:          prior NIG parameter for C - python float
    :param nu_C:           prior NIG parameter for C - python float
    :param alpha_C:        prior NIG parameter for C - python float
    :param beta_C:         prior NIG parameter for C - python float

    :return:               labels - classification of x_test, numpy array (n, ) int32, values 0 or 1
    """
    pdf_A = bayes_estim_pdf_normal(x_test, x_train_A, mu0_A, nu_A, alpha_A, beta_A)
    prior_A = len(x_train_A) / (len(x_train_A) + len(x_train_C))
    pdf_C = bayes_estim_pdf_normal(x_test, x_train_C, mu0_C, nu_C, alpha_C, beta_C)
    prior_C = len(x_train_C) / (len(x_train_A) + len(x_train_C))

    return np.int32(prior_A * pdf_A <= prior_C * pdf_C)


################################################################################
#####                                                                      #####
#####                Put functions from previous labs here.                #####
#####            (Sorry, we know imports would be much better)             #####
#####                                                                      #####
################################################################################


def find_strategy_2normal(distribution_A, distribution_B):
    """
    q = find_strategy_2normal(distribution_A, distribution_B)

    Find optimal bayesian strategy for 2 normal distributions and zero-one loss function.

    :param distribution_A:  parameters of the normal dist.
                            distribution_A['Mean'], distribution_A['Sigma'], distribution_A['Prior'] - python floats
    :param distribution_B:  the same as distribution_A

    :return q:              strategy dict
                               q['t1'], q['t2'] - decision thresholds - python floats
                               q['decision'] - (3, ) np.int32 np.array decisions for intervals (-inf, t1>, (t1, t2>, (t2, inf)
                               If there is only one threshold, q['t1'] should be equal to q['t2'] and the middle decision should be 0
                               If there is no threshold, q['t1'] and q['t2'] should be -/+ infinity and all the decision values should be the same (0 preferred)
    """
    s_A = distribution_A['Sigma']
    m_A = distribution_A['Mean']
    p_A = distribution_A['Prior']
    s_B = distribution_B['Sigma']
    m_B = distribution_B['Mean']
    p_B = distribution_B['Prior']

    q = dict()

    # extreme priors
    eps = 1e-10
    if p_A < eps:
        q['t1'] = -np.inf
        q['t2'] = np.inf
        q['decision'] = np.int32(np.repeat(1, 3))
    elif p_B < eps:
        q['t1'] = -np.inf
        q['t2'] = np.inf
        q['decision'] = np.int32(np.repeat(0, 3))
    else:
        a = s_A ** 2 - s_B ** 2
        b = 2 * s_B ** 2 * m_A - 2 * s_A ** 2 * m_B
        c = - s_B ** 2 * m_A ** 2 + s_A ** 2 * m_B ** 2 - np.log(p_B / p_A * s_A / s_B) * 2 * s_A ** 2 * s_B ** 2

        if a == 0:
            # same sigmas -> not quadratic
            if b == 0:
                # same sigmas and same means -> not even linear
                q['t1'] = -np.inf
                q['t2'] = np.inf
                q['decision'] = np.int32(np.repeat(int(not c >= 0), 3))
            else:
                # same sigmas, different means -> linear equation
                q['t1'] = q['t2'] = - c / b
                q['decision'] = np.int32([1, 0, 0]) if b > 0 else np.int32([0, 0, 1])
        else:
            # quadratic equation
            D = b ** 2 - 4 * a * c

            if D > 0:
                q['t1'] = (-b - np.sqrt(D)) / (2 * a)
                q['t2'] = (-b + np.sqrt(D)) / (2 * a)
                q['decision'] = np.int32([0, 1, 0]) if a > 0 else np.int32([1, 0, 1])
            elif D == 0:
                q['t1'] = q['t2'] = -b / (2 * a)
                q['decision'] = np.int32([0, 1, 0]) if a > 0 else np.int32([1, 0, 1])
            elif D < 0:
                q['t1'] = -np.inf
                q['t2'] = np.inf
                q['decision'] = np.int32(np.repeat(int(not a > 0), 3))

    q['t1'] = float(q['t1'])
    q['t2'] = float(q['t2'])

    if q['t1'] > q['t2']:
        t = q['t1']
        q['t1'] = q['t2']
        q['t2'] = t

    return q


def classify_2normal(measurements, q):
    """
    label = classify_2normal(measurements, q)

    Classify images using continuous measurements and strategy q.

    :param imgs:    test set measurements, np.array (n, )
    :param q:       strategy
                    q['t1'] q['t2'] - float decision thresholds
                    q['decision'] - (3, ) int32 np.array decisions for intervals (-inf, t1>, (t1, t2>, (t2, inf)
    :return:        label - classification labels, (n, ) int32
    """

    def decide(measurement):
        return q['decision'][0] if measurement < q['t1'] else q['decision'][1] if measurement < q['t2'] else \
        q['decision'][2]

    return np.array([decide(m) for m in measurements], dtype=np.int32)


def classification_error(predictions, labels):
    """
    error = classification_error(predictions, labels)

    :param predictions: (n, ) np.array of values 0 or 1 - predicted labels
    :param labels:      (n, ) np.array of values 0 or 1 - ground truth labels
    :return:            error - classification error ~ a fraction of predictions being incorrect
                        python float in range <0, 1>
    """
    return float(np.sum(predictions != labels) / len(labels))


################################################################################
#####                                                                      #####
#####             Below this line are already prepared methods             #####
#####                                                                      #####
################################################################################


def compute_measurement_lr_cont(imgs):
    """
    x = compute_measurement_lr_cont(imgs)

    Compute measurement on images, subtract sum of right half from sum of
    left half.

    :param imgs:    set of images, (h, w, n) numpy array
    :return x:      measurements, (n, ) numpy array
    """
    assert len(imgs.shape) == 3

    width = imgs.shape[1]
    sum_rows = np.sum(imgs, dtype=np.float64, axis=0)

    x = np.sum(sum_rows[0:int(width / 2),:], axis=0) - np.sum(sum_rows[int(width / 2):,:], axis=0)

    assert x.shape == (imgs.shape[2], )
    return x


def mle_likelihood_normal(x, mu, var):
    """
    Compute the likelihood of the data x given the model is a normal distribution with given mean and sigma

    :param x:       measurements, numpy array (n, )
    :param mu:      the normal distribution mean
    :param var:     the normal distribution variance
    :return:        L - likelihood of the data x
    """
    assert len(x.shape) == 1

    if var <= 0:
        L = 0
    else:
        L = np.prod(norm.pdf(x, mu, np.sqrt(var)))
    return L


def norm_inv_gamma_pdf(mu, var, mu0, nu, alpha, beta):
    # Wikipedia sometimes uses a symbol 'lambda' instead 'nu'

    assert alpha > 0
    assert nu > 0
    if beta <= 0 or var <= 0:
        return 0

    sigma = np.sqrt(var)

    p = np.sqrt(nu) / (sigma * np.sqrt(2 * np.pi)) * np.power(beta, alpha) / spec.gamma(alpha) * np.power(1/var, alpha + 1) * np.exp(-(2 * beta + nu * (mu0 - mu) * (mu0 - mu)) / (2 * var))

    return p
