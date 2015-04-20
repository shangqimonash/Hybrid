import rag_lil
import numpy as np
import random


class Graph(object):

    def __init__(self, n):

        self.rows = np.empty((n,), dtype=object)
        self.data = np.empty((n,), dtype=object)
        for i in range(n):
            self.data[i] = np.empty(0, dtype=np.int)
            self.rows[i] = np.empty(0, dtype=np.int)

        self.vertex_count = n
        # mean_color_array[i] should give mean color of region i
        self.mean_color_array = np.zeros(n)

    def display(self):
        for i in range(len(self.rows)):
            d = len(self.rows[i])
            idx = 0
            while idx < d and self.rows[i][idx] < i:
                print "(%d , %d) -> %d" % (i, self.rows[i][idx], self.data[i][idx])
                idx += 1

    def make_edge(self, i, j, wt):
        rag_lil.add_edge_py(self.rows, self.data, i, j, wt)

    def merge(self, i, j):
        rag_lil.merge_node_py(self.rows, self.data, i, j)

    def draw(self, name):
        g = pgv.AGraph()
        for i in range(self.rows.shape[0]):
            g.add_node(i)

        for i in range(self.rows.shape[0]):
            for j in range(self.rows[i].shape[0]):
                g.add_edge(i, self.rows[i][j])
                e = g.get_edge(i, self.rows[i][j])
                e.attr['label'] = str(self.data[i][j])

        g.layout('circo')
        g.draw(name)

    def random_merge(self, minimum):
        n = self.vertex_count
        while n > minimum:

            i = random.randint(0, self.vertex_count - 1)
            if self.rows[i].shape[0] > 0:
                k = random.choice(self.rows[i])
                self.merge(i, k)
                n -= 1


def construct_rag(arr):
    return rag_lil.construct_rag_3d_lil(arr)
