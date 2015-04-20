# cython: profile=True

import numpy as np
cimport numpy as cnp
from scipy import sparse
import graph_csr
cimport cython


@cython.profile(False)
cdef inline max(cnp.int32_t a, cnp.int32_t b):
    if a > b:
        return a
    else:
        return b


def merge(g, cnp.int32_t x, cnp.int32_t y):
    cdef cnp.int32_t[:] indptr_view = g.indptr
    cdef cnp.int32_t[:] indices_view = g.indices
    cdef cnp.int64_t[:] data_view = g.data
    cdef cnp.int32_t[:] new_row_view
    cdef cnp.int64_t[:] new_data_view
    cdef Py_ssize_t i, j, n
    cdef cnp.int32_t[:] tmp_row_view
    cdef cnp.int64_t[:] tmp_data_view
    cdef cnp.int32_t w1, w2, row, w

    new_row = np.union1d(
        indices_view[indptr_view[x]:indptr_view[x + 1]],
        indices_view[indptr_view[y]:indptr_view[y + 1]])

    new_row = new_row[new_row != -1]
    new_row = new_row[new_row != x]
    new_row = new_row[new_row != y]
    new_row_view = new_row

    if(new_row_view.shape[0] == 0):
        raise ValueError

    new_data = np.zeros_like(new_row, dtype=np.int64)
    new_data -= 1

    new_data_view = new_data

    i = 0

    # print "new row view =",np.array(new_row_view)
    while i < new_row_view.shape[0]:

        row = new_row_view[i]
        tmp_row_view = indices_view[indptr_view[row]:indptr_view[row + 1]]
        tmp_data_view = data_view[indptr_view[row]:indptr_view[row + 1]]

        j = 0
        w1 = -1
        w2 = -1

        # arr = np.array(tmp_row_view)
        # a = np.where(arr == x)[0].shape[0]
        # b = np.where(arr == y)[0].shape[0]
        # if a == 0 and b ==0 :
        #    raise ValueError
        while j < indptr_view[row + 1] - indptr_view[row]:
            # if tmp_row_view[j] == -1 :
            #    continue

            if tmp_row_view[j] == x:

                w1 = tmp_data_view[j]
                indices_view[indptr_view[row] + j] = g.num_rows

            if tmp_row_view[j] == y:

                w2 = tmp_data_view[j]
                indices_view[indptr_view[row] + j] = g.num_rows

            w = max(w1, w2)
            data_view[row + j] = w
            new_data_view[i] = w

            j += 1

        i += 1

    n = new_row_view.shape[0]
    i = g.array_size

    data_view[i:i + n] = new_data_view
    indices_view[i:i + n] = new_row_view
    indptr_view[g.num_rows] = i
    indptr_view[g.num_rows + 1] = i + n

    g.valid[g.num_rows] = True
    g.valid[x] = False
    g.valid[y] = False

    g.num_rows += 1
    g.array_size = g.array_size + n


@cython.profile(False)
def construct_rag_3d_csr(cnp.int32_t[:, :, :] arr):
    cdef Py_ssize_t l, b, h, i, j, k
    cdef cnp.int32_t current, next

    l = arr.shape[0]
    b = arr.shape[1]
    h = arr.shape[2]

    n = np.amax(arr) + 1
    g = sparse.dok_matrix((n, n), dtype=int)

    i = 0
    while i < l - 1:
        # print i
        j = 0
        while j < b - 1:
            k = 0
            while k < h - 1:
                current = arr[i, j, k]

                next = arr[i + 1, j, k]
                if current != next:
                    g[current, next] = 1
                    g[next, current] = 1

                next = arr[i, j + 1, k]
                if current != next:
                    g[current, next] = 1
                    g[next, current] = 1

                next = arr[i + 1, j + 1, k]
                if current != next:
                    g[current, next] = 1
                    g[next, current] = 1

                next = arr[i + 1, j, k + 1]
                if current != next:
                    g[current, next] = 1
                    g[next, current] = 1

                next = arr[i, j + 1, k + 1]
                if current != next:
                    g[current, next] = 1
                    g[next, current] = 1

                next = arr[i + 1, j + 1, k + 1]
                if current != next:
                    g[current, next] = 1
                    g[next, current] = 1

                next = arr[i, j, k + 1]
                if current != next:
                    g[current, next] = 1
                    g[next, current] = 1

                k += 1

            j += 1

        i += 1

    return graph_csr.Graph(g)
