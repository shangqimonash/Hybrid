import networkx as nx
import rag_nx
import random

random.seed(1)


class Graph(nx.Graph):

    def __init__(self, n):
        nx.Graph.__init__(self)
        self.add_nodes_from(range(n))
        self.vertex_count = n

    def display(self):
        for i in self.nodes():
            # print i,self.neighbors(i)
            for j in self.neighbors(i):
                if j < i:
                    d = self.get_edge_data(i, j)
                    print "(%d,%d) -> %d" % (i, j, d['weight'])

    def make_edge(self, i, j, wt):
        self.add_edge(i, j, weight=wt)

    #@profile
    def merge(self, i, j):

        if not self.has_edge(i, j):
            raise ValueError('Cant merge non adjacent nodes')

        # print "before ",self.order()
        for x in self.neighbors(i):
            if x == j:
                continue
            w1 = self.get_edge_data(x, i)['weight']
            w2 = -1
            if self.has_edge(x, j):
                w2 = self.get_edge_data(x, j)['weight']

            w = max(w1, w2)

            self.add_edge(x, j, weight=w)

        self.remove_node(i)
        # print "after",self.order()

    def random_merge(self, minimum):

        n = self.vertex_count
        while n > minimum:

            i = random.randint(0, self.vertex_count - 1)
            if self.has_node(i) and len(self.neighbors(i)) > 0:
                k = random.choice(self.neighbors(i))
                self.merge(i, k)
                n -= 1


def construct_rag(arr):
    return rag_nx.construct_rag_3d_nx(arr)
