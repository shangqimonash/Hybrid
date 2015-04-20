import pygraphviz as pgv
import random
import rag_custom
random.seed(1)


class Graph(object):

    def __init__(self, n):

        # TODO : Why shoukd this be a list, can't this be a dict as well
        # Number of vertices do not remain the same
        self.rows = [{} for i in range(n)]
        self.edge_count = 0
        self.vertex_count = n
        self.prop = {}

    def display(self):
        for i in range(len(self.rows)):
            for key in self.rows[i]:
                if key < i:
                    print "(%d,%d) -> %d" % (i, key, self.rows[i][key])

    @profile
    def make_edge(self, i, j, wt):
        try:
            self.rows[i][j]
        except KeyError:
            self.edge_count += 1

        self.rows[i][j] = wt
        self.rows[j][i] = wt

    def neighbors(self, i):
        return self.rows[i].keys()

    def get_weight(self, i, j):
        return self.rows[i][j]

    @profile
    def merge(self, i, j):

        if not self.has_edge(i, j):
            raise ValueError('Cant merge non adjacent nodes')

        # print "before ",self.order()
        for x in self.neighbors(i):
            if x == j:
                continue
            w1 = self.get_weight(x, i)
            w2 = -1
            if self.has_edge(x, j):
                w2 = self.get_weight(x, j)

            w = max(w1, w2)

            self.make_edge(x, j, w)

        self.remove_node(i)
        # print "after",self.order()

    def draw(self, name):
        g = pgv.AGraph()

        for i in range(len(self.rows)):
            for key in self.rows[i]:
                if key < i:
                    g.add_edge(i, key)
                    e = g.get_edge(i, key)
                    e.attr['label'] = str(self.rows[i][key])

        g.layout('circo')
        g.draw(name)

    def has_edge(self, i, j):
        try:
            self.rows[i][j]
            return True
        except KeyError:
            return False

    def remove_node(self, x):
        for i in self.neighbors(x):
            del self.rows[i][x]

        self.rows[x] = {}

    def random_merge(self, minimum):

        n = self.vertex_count
        while n > minimum:
            i = random.randint(0, self.vertex_count - 1)
            if len(self.rows[i]) > 0:
                k = random.choice(self.rows[i].keys())
                self.merge(i, k)
                n -= 1


def construct_rag(arr):
    return rag_custom.construct_rag_3d_custom(arr)
