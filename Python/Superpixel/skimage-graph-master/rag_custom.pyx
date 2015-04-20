import graph_custom
cimport numpy as cnp
import numpy as np


def construct_rag_3d_custom(cnp.int32_t[:, :, :] arr):
    cdef Py_ssize_t l, b, h, i, j, k
    cdef cnp.int32_t current, next
    l = arr.shape[0]
    b = arr.shape[1]
    h = arr.shape[2]

    g = graph_custom.Graph(np.amax(arr) + 1)

    i = 0
    while i < l - 1:
        j = 0
        while j < b - 1:
            k = 0
            while k < h - 1:
                current = arr[i, j, k]

                next = arr[i + 1, j, k]
                if current != next:
                    g.make_edge(current, next, 1)

                next = arr[i, j + 1, k]
                if current != next:
                    g.make_edge(current, next, 1)

                next = arr[i + 1, j + 1, k]
                if current != next:
                    g.make_edge(current, next, 1)

                next = arr[i + 1, j, k + 1]
                if current != next:
                    g.make_edge(current, next, 1)

                next = arr[i, j + 1, k + 1]
                if current != next:
                    g.make_edge(current, next, 1)

                next = arr[i + 1, j + 1, k + 1]
                if current != next:
                    g.make_edge(current, next, 1)

                next = arr[i, j, k + 1]
                if current != next:
                    g.make_edge(current, next, 1)

                k += 1

            j += 1

        i += 1

    return g
