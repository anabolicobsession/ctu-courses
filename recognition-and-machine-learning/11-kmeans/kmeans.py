#!/usr/bin/python
# -*- coding: utf-8 -*-

import numpy as np
import matplotlib.pyplot as plt
import itertools


def assign_centroids(X, centroids):
    difference_tensor = np.expand_dims(X, 2) - np.expand_dims(centroids, 1)
    norms = np.linalg.norm(difference_tensor, axis=0)
    return np.argmin(norms, axis=1)


def k_means(X, K, max_iter, show=False, init_means=None):
    """
    Implementation of the k-means clustering algorithm.

    :param X:               feature vectors, np array (dim, number_of_vectors)
    :param K:               required number of clusters, scalar
    :param max_iter:        stopping criterion: max. number of iterations
    :param show:            (optional) boolean switch to turn on/off visualization of partial results
    :param init_means:      (optional) initial cluster prototypes, np array (dim, k)

    :return cluster_labels: cluster index for each feature vector, np array (number_of_vectors, )
                            array contains only values from 0 to k-1,
                            i.e. cluster_labels[i] is the index of a cluster which the vector x[:,i] belongs to.
    :return centroids:      cluster centroids, np array (dim, k), same type as x
                            i.e. centroids[:,i] is the center of the i-th cluster.
    :return sq_dists:       squared distances to the nearest centroid for each feature vector,
                            np array (number_of_vectors, )

    Note 1: The iterative procedure terminates if either maximum number of iterations is reached
            or there is no change in assignment of data to the clusters.

    Note 2: DO NOT MODIFY INITIALIZATIONS

    """
    n = X.shape[1]
    X_centroids = np.zeros(n, np.int32)

    if init_means is None:
        ids = np.random.choice(n, K, replace=False)
        centroids = X[:, ids]
    else:
        centroids = init_means

    i = 0
    X_centroids_prev = None

    while i < max_iter:
        X_centroids = assign_centroids(X, centroids)
        for k in range(K):
            centroids[:, k] = np.average(X[:, X_centroids == k], axis=1)

        if show:
            print('Iteration: {:d}'.format(i))
            show_clusters(X, X_centroids, centroids, title='Iteration: {:d}'.format(i))

        if np.all(X_centroids == X_centroids_prev):
            break

        i += 1
        X_centroids_prev = np.copy(X_centroids)

    if show:
        print('Done.')

    return X_centroids, centroids, np.square(np.linalg.norm(X - centroids[:, X_centroids], axis=0))


def k_means_multiple_trials(X, K, trials, max_iter, show=False):
    """
    Performs several trials of the k-centroids clustering algorithm in order to
    avoid local minima. Result of the trial with the lowest "within-cluster
    sum of squares" is selected as the best one and returned.

    :param X:               feature vectors, np array (dim, number_of_vectors)
    :param K:               required number of clusters, scalar
    :param trials:          number of trials, scalars
    :param max_iter:        stopping criterion: max. number of iterations
    :param show:            (optional) boolean switch to turn on/off visualization of partial results

    :return cluster_labels: cluster index for each feature vector, np array (number_of_vectors, ),
                            array contains only values from 0 to k-1,
                            i.e. cluster_labels[i] is the index of a cluster which the vector x[:,i] belongs to.
    :return centroids:      cluster centroids, np array (dim, k), same type as x
                            i.e. centroids[:,i] is the center of the i-th cluster.
    :return sq_dists:       squared distances to the nearest centroid for each feature vector,
                            np array (number_of_vectors, )
    """
    best_kmeans = (None, None, None)
    min_loss = np.inf

    for _ in range(trials):
        kmeans = k_means(X, K, max_iter, show)
        loss = np.sum(kmeans[2])

        if loss < min_loss:
            min_loss = loss
            best_kmeans = kmeans

    return best_kmeans


def random_sample(weights):
    """
    picks randomly a sample based on the sample weights.

    :param weights: array of sample weights, np array (n, )
    :return idx:    index of chosen sample, scalar

    Note: use np.random.uniform() for random number generation in open interval (0, 1)
    """
    return np.random.choice(np.arange(len(weights)), p=weights / np.sum(weights))


def k_meanspp(X, K):
    """
    performs k-means++ initialization for k-means clustering.

    :param X:           Feature vectors, np array (dim, number_of_vectors)
    :param K:           Required number of clusters, scalar

    :return centroids:  proposed centroids for k-means initialization, np array (dim, k)
    """
    n = X.shape[1]
    centroids = X[:, random_sample(np.ones(n))].reshape(-1, 1)

    for i in range (1, K):
        X_centroids = assign_centroids(X, centroids)
        squared_distances = np.square(np.linalg.norm(X - centroids[:, X_centroids], axis=0))
        new_centroid = X[:, random_sample(squared_distances)].reshape(-1, 1)
        centroids = np.concatenate([centroids, new_centroid], axis=1)

    return centroids


def quantize_colors(image, K):
    """
    Image color quantization using the k-means clustering. A subset of 1000 pixels
    is first clustered into k clusters based on their RGB color.
    Quantized image is constructed by replacing each pixel color by its cluster centroid.

    :param image:          image for quantization, np array (h, w, 3) (np.uint8)
    :param K:           required number of quantized colors, scalar
    :return im_q:       image with quantized colors, np array (h, w, 3) (uint8)
    
    note: make sure that the k-means is run on floating point inputs.
    """
    height, width, _ = image.shape
    pixels = image.astype(np.float64).reshape(-1, 3).T
    ids = np.random.randint(0, (height * width) - 1, 1000)
    centroids = k_means(pixels[:, ids], K, np.inf)[1]
    return centroids[:, assign_centroids(pixels, centroids)].T.reshape(image.shape).astype('uint8')


################################################################################
#####                                                                      #####
#####             Below this line are already prepared methods             #####
#####                                                                      #####
################################################################################


def compute_measurements(images):
    """
    computes 2D features from image measurements

    :param images: array of images, np array (H, W, N_images) (np.uint8)
    :return x:     array of features, np array (2, N_images)
    """

    images = images.astype(np.float64)
    H, W, N = images.shape

    left = images[:, :(W//2), :]
    right = images[:, (W//2):, :]
    up = images[:(H//2), ...]
    down = images[(H//2):, ...]

    L = np.sum(left, axis=(0, 1))
    R = np.sum(right, axis=(0, 1))
    U = np.sum(up, axis=(0, 1))
    D = np.sum(down, axis=(0, 1))

    a = L - R
    b = U - D

    x = np.vstack((a, b))
    return x


def show_clusters(x, cluster_labels, centroids, title=None):
    """
    Create plot of feature vectors with same colour for members of same cluster.

    :param x:               feature vectors, np array (dim, number_of_vectors) (float64/double),
                            where dim is arbitrary feature vector dimension
    :param cluster_labels:  cluster index for each feature vector, np array (number_of_vectors, ),
                            array contains only values from 1 to k,
                            i.e. cluster_labels[i] is the index of a cluster which the vector x[:,i] belongs to.
    :param centroids:       cluster centers, np array (dim, k) (float64/double),
                            i.e. centroids[:,i] is the center of the i-th cluster.
    """

    cluster_labels = cluster_labels.flatten()
    clusters = np.unique(cluster_labels)
    markers = itertools.cycle(['*','o','+','x','v','^','<','>'])

    plt.figure(figsize=(8,7))
    for i in clusters:
        cluster_x = x[:, cluster_labels == i]
        # print(cluster_x)
        plt.plot(cluster_x[0], cluster_x[1], next(markers))
    plt.axis('equal')

    len = centroids.shape[1]
    for i in range(len):
        plt.plot(centroids[0, i], centroids[1, i], 'm+', ms=10, mew=2)

    plt.axis('equal')
    plt.grid('on')
    if title is not None:
        plt.title(title)


def show_clustered_images(images, labels, title=None):
    """
    Shows results of clustering. Create montages of images according to estimated labels

    :param images:          input images, np array (h, w, n)
    :param labels:          labels of input images, np array (n, )
    """
    assert (len(images.shape) == 3)

    labels = labels.flatten()
    l = np.unique(labels)
    n = len(l)

    def montage(images, colormap='gray'):
        h, w, count = np.shape(images)
        h_sq = np.int(np.ceil(np.sqrt(count)))
        w_sq = h_sq
        im_matrix = np.zeros((h_sq * h, w_sq * w))

        image_id = 0
        for j in range(h_sq):
            for k in range(w_sq):
                if image_id >= count:
                    break
                slice_w = j * h
                slice_h = k * w
                im_matrix[slice_h:slice_h + w, slice_w:slice_w + h] = images[:, :, image_id]
                image_id += 1
        plt.imshow(im_matrix, cmap=colormap)
        plt.axis('off')
        return im_matrix

    unique_labels = np.unique(labels).flatten()

    fig = plt.figure(figsize=(10,10))
    ww = np.ceil(float(n) / np.sqrt(n))
    hh = np.ceil(float(n) / ww)

    for i in range(n):
        imgs = images[:, :, labels == unique_labels[i]]
        subfig = plt.subplot(hh,ww,i+1)
        montage(imgs)

    if title is not None:
        fig.suptitle(title)


def show_mean_images(images, labels, letters=None, title=None):
    """
    show_mean_images(images, c)

    Compute mean image for a cluster and show it.

    :param images:          input images, np array (h, w, n)
    :param labels:          labels of input images, np array (n, )
    :param letters:         labels for mean images, string/array of chars
    """
    assert (len(images.shape) == 3)

    labels = labels.flatten()
    l = np.unique(labels)
    n = len(l)

    unique_labels = np.unique(labels).flatten()

    fig = plt.figure()
    ww = np.ceil(float(n) / np.sqrt(n))
    hh = np.ceil(float(n) / ww)

    for i in range(n):
        imgs = images[:, :, labels == unique_labels[i]]
        img_average = np.squeeze(np.average(imgs.astype(np.float64), axis=2))
        subfig = plt.subplot(hh,ww,i+1)
        plt.imshow(img_average, cmap='gray')
        if letters is not None:
            plt.title(letters[i])
        plt.axis('off')

    if title is not None:
        fig.suptitle(title)



def gen_kmeanspp_data(mu=None, sigma=None, n=None):
    """
    generates data with n_clusterss normally distributed clusters

    It generates 4 clusters with 80 points by default.

    :param mu:          mean of normal distribution, np array (dim, n_clusters)
    :param sigma:       std of normal distribution, scalar
    :param n:           number of output points for each distribution, scalar
    :return samples:    dim-dimensional samples with n samples per cluster, np array (dim, n_clusters * n)
    """

    sigma = 1. if sigma is None else sigma
    mu = np.array([[-5, 0], [5, 0], [0, -5], [0, 5]]) if mu is None else mu
    n = 80 if n is None else n

    samples = np.random.normal(np.tile(mu, (n, 1)).T, sigma)
    return samples
