# cython: profile=True

import numpy as np
cimport numpy as cnp
import pygraphviz as pgv
import graph_lil
cimport cython


@cython.profile(False)
cdef inline max(cnp.int_t a, cnp.int_t b):
    if a > b:
        return a
    else:
        return b


cdef add_edge(object[:] rows, object[:] data, Py_ssize_t i, Py_ssize_t j, cnp.int_t wt):
    cdef Py_ssize_t index

    index = np.searchsorted(rows[i], j)
    if index < rows[i].shape[0] and rows[i][index] == j:
        data[i][index] == wt
        index = np.searchsorted(rows[j], i)
        data[j][index] = wt

        return

    rows[i] = np.insert(rows[i], index, j)
    data[i] = np.insert(data[i], index, wt)

    index = np.searchsorted(rows[j], i)
    rows[j] = np.insert(rows[j], index, i)
    data[j] = np.insert(data[j], index, wt)


@cython.profile(False)
def add_edge_py(rows, data, i, j, w):
    add_edge(rows, data, i, j, w)


@cython.profile(False)
def merge_node_py(rows, data, x, y):
    merge_node(rows, data, x, y)

# assumes nodes have edge between them


cdef merge_node(object[:] rows, object[:] data, cnp.int_t x, cnp.int_t y):

    # disconnect all nodes connected to i and connect them to j
    # the adjacency list of i becaomes empty

    cdef Py_ssize_t n1, n2, n, i = 0, j = 0, idx = 0, idx_of_x, idx_of_y
    cdef cnp.int_t wt_x, wt_y, v, weight
    n1 = rows[x].shape[0]
    n2 = rows[y].shape[0]

    # maximum size of adjacency list of new node
    n = n1 + n2

    new_row = np.zeros(n, dtype=np.int)
    new_data = np.zeros(n, dtype=np.int)

    # the row and data for the new node
    cdef cnp.int_t[::1] new_row_view = new_row
    cdef cnp.int_t[::1] new_data_view = new_data

    cdef cnp.int_t[::1] rowx = rows[x]
    cdef cnp.int_t[::1] rowy = rows[y]

    # till both the rows to be merged are exhausted
    # `idx` stored number of enteries in new_row_view
    # `i` iterates over `rowx`
    # `j` iterates over `rowy`
    while i < n1 or j < n2:

        # rowx is over, just copy rest of rowy
        if i >= n1:
            while j < n2:
                # skip copying j
                if rowy[j] == x:
                    j += 1
                    continue
                new_row_view[idx] = rowy[j]
                j += 1
                idx += 1
            break

        # rowy is over, copy rest of rowx
        if j >= n2:
            while i < n1:
                # skip copying i
                if rowx[i] == y:
                    i += 1
                    continue
                new_row_view[idx] = rowx[i]
                i += 1
                idx += 1
            break

        # do not copy i or j
        if rowx[i] == y:
            i += 1
            continue

        if rowy[j] == x:
            j += 1
            continue

        # merge in increasing order
        if rowx[i] < rowy[j]:
            new_row_view[idx] = rowx[i]
            i += 1
            idx += 1
        elif rowx[i] > rowy[j]:
            new_row_view[idx] = rowy[j]
            j += 1
            idx += 1
        else:
            # duplicate values, increment both counters
            new_row_view[idx] = rowy[j]
            idx += 1
            i += 1
            j += 1

    # slice only the copied values
    n = idx
    new_row_view = new_row_view[0:n]
    new_data_view = new_data_view[0:n]

    idx = 0
    while idx < n:

        v = new_row_view[idx]
        wt_x = wt_y = -1

        # search if node was connected to `x`
        # if yes, delete the link to `x` and
        # record the weight

        idx_of_x = np.searchsorted(rows[v], x)

        if idx_of_x < rows[v].shape[0] and rows[v][idx_of_x] == x:

            # node `x` was there in the list
            # we don't need no `x`
            wt_x = data[v][idx_of_x]

            rows[v] = np.delete(rows[v], idx_of_x)
            data[v] = np.delete(data[v], idx_of_x)

        # search if node was connected to `y`
        # if yes record the weight
        idx_of_y = np.searchsorted(rows[v], y)

        if idx_of_y < rows[v].shape[0] and rows[v][idx_of_y] == y:
            #`y` was in the list... Peace ! :)
            wt_y = data[v][idx_of_y]
            weight = max(wt_x, wt_y)

            # we just need to update the weight in both places
            data[v][idx_of_y] = weight
            new_data_view[idx] = weight

        else:
            # `y` ins't in the list

            weight = wt_x
            # insert `y` where it should be
            rows[v] = np.insert(rows[v], idx_of_y, y)

            # insert the weight where it should go
            data[v] = np.insert(data[v], idx_of_y, weight)
            # update weight in new row
            new_data_view[idx] = weight
        idx += 1

    rows[y] = np.array(new_row_view)
    data[y] = np.array(new_data_view)

    rows[x] = np.empty(0, dtype=np.int)
    data[x] = np.empty(0, dtype=np.int)


@cython.profile(False)
def construct_rag_3d_lil(cnp.int32_t[:, :, :] arr):
    cdef Py_ssize_t l, b, h, i, j, k
    cdef cnp.int32_t current, next
    l = arr.shape[0]
    b = arr.shape[1]
    h = arr.shape[2]

    g = graph_lil.Graph(np.amax(arr) + 1)

    i = 0
    while i < l - 1:
        j = 0
        print i
        while j < b - 1:
            k = 0
            while k < h - 1:
                current = arr[i, j, k]

                next = arr[i + 1, j, k]
                if current != next:
                    add_edge(g.rows, g.data, current, next, 1)

                next = arr[i, j + 1, k]
                if current != next:
                    add_edge(g.rows, g.data, current, next, 1)

                next = arr[i + 1, j + 1, k]
                if current != next:
                    add_edge(g.rows, g.data, current, next, 1)

                next = arr[i + 1, j, k + 1]
                if current != next:
                    add_edge(g.rows, g.data, current, next, 1)

                next = arr[i, j + 1, k + 1]
                if current != next:
                    add_edge(g.rows, g.data, current, next, 1)

                next = arr[i + 1, j + 1, k + 1]
                if current != next:
                    add_edge(g.rows, g.data, current, next, 1)

                next = arr[i, j, k + 1]
                if current != next:
                    add_edge(g.rows, g.data, current, next, 1)

                k += 1

            j += 1

        i += 1

    return g
