from scipy import sparse
import numpy as np
import rag_csr
import random
random.seed(1)


class Graph(object):

    def __init__(self, arr):
        arr = sparse.csr_matrix(arr)
        self.data = np.zeros((arr.data.shape[0] * 2,), dtype=arr.data.dtype)
        self.indices = np.zeros(
            (arr.indices.shape[0] * 2,
             ),
            dtype=arr.indices.dtype)
        self.indptr = np.zeros(
            (arr.indptr.shape[0] * 2,
             ),
            dtype=arr.indptr.dtype)
        self.valid = np.zeros((arr.indptr.shape[0] * 2,), dtype=bool)

        self.data[0:arr.data.shape[0]] = arr.data
        self.indices[0:arr.indices.shape[0]] = arr.indices
        self.indptr[0:arr.indptr.shape[0]] = arr.indptr

        self.valid[0:arr.indptr.shape[0]] = True
        self.array_size = arr.indices.shape[0]
        self.max_size = self.indices.shape[0]
        self.num_rows = arr.shape[0]

        # print self.data
        # print self.indices
        # print self.indptr
        # print self.valid
        self.vertex_count = arr.shape[0]

    def display(self):
        for i in range(self.indptr.shape[0]):
            if self.valid[i]:

                start = self.indptr[i]
                end = self.indptr[i + 1]

                while start < end:
                    if self.indices[start] < i and self.indices[start] >= 0:
                        if self.indices[start] >= 0:
                            print "(%d,%d) -> %d" % (i, self.indices[start], self.data[start])
                    start += 1

    def double(self):
        new_indices = np.zeros(
            (self.indices.shape[0] * 2,
             ),
            dtype=self.indices.dtype)
        new_data = np.zeros((self.data.shape[0] * 2,), dtype=self.data.dtype)

        new_indices[:self.indices.shape[0]] = self.indices
        new_data[:self.data.shape[0]] = self.data
        self.indices = new_indices
        self.data = new_data
        self.max_size = self.data.shape[0]

    def debug(self):
        print "indptr", self.indptr
        print "indices", self.indices
        print "data", self.data

    def merge(self, x, y):
        len_of_x = self.indptr[x + 1] - self.indptr[x]
        len_of_y = self.indptr[y + 1] - self.indptr[y]

        # In most cases executed only once
        while self.array_size + len_of_x + len_of_y > self.max_size:
            self.double()

        rag_csr.merge(self, x, y)

    def random_merge(self, minimum):
        n = self.vertex_count
        while n > minimum:
            r = random.randint(0, self.num_rows - 2)
            if self.valid[r]:
                # print n

                arr = self.indices[self.indptr[r]: self.indptr[r + 1]]
                if arr.shape[0] == 0:
                    continue
                x = random.choice(arr)
                # print x,r
                self.merge(x, r)
                # print np.where(self.valid)[0].shape
                n -= 1


def construct_rag(arr):
    return rag_csr.construct_rag_3d_csr(arr)
