# -*- coding: utf-8 -*-

'''
构建我的RAG 数据结构

graph[n]直接index是access node n出发的 edge可以直接用graph[n][3]['weight]这样三个index来取得
也可以，graph[n].get(3)[weight]来取得，那个get其实是dictionary的一个函数

而你要访问node的花，就要用graph.node[n]来访问，这就是edge和node的区别
'''
import networkx as nx

class RAG(nx.Graph):

    """
    The Region Adjacency Graph (RAG) of an image, subclasses
    `networx.Graph <http://networkx.github.io/documentation/latest/reference/classes.graph.html>`_
    """

    def __init__(self, data=None, **attr):

        super(RAG, self).__init__(data, **attr)
        try:
            #max(self.nodes_iter())求出的是你add_node(n)中n最大的编号，如果你仅仅add一个node 9，
            #这里就是9，但是你没有办法访问G.node[8]，因为它还没有加入，但是这里的max_id就是9
            self.max_id = max(self.nodes_iter())
        except ValueError:
            # Empty sequence
            self.max_id = 0

    def freqUpdate(self):
        '''
        根据数值的histogram，更新所有node的frequency histogram
        :return:
        '''
        allNode=self.nodes()
        for nodeID in allNode:
            self.node[nodeID]['freqHist']=1.0*self.node[nodeID]['hist']/\
                self.node[nodeID]['pixelCount']


    def merge_nodes(self, src, dst, in_place=True):
        """Merge node `src` and `dst`.

        The new combined node is adjacent to all the neighbors of `src`
        and `dst`.

        融合仅仅更新node的histogram，

        edge不用更新，因为我们的heap处理是在处理外面的那个edge，融合之后的edge 的weight，就是neighbor
        和des的weight，如果这个neighbor和dest没有edge，那么融合后的edge weight就是和src的weight

        新node的id时dest的id


        Parameters
        ----------
        src, dst : int
            Nodes to be merged.
        in_place : bool, optional
            If set to `True`, the merged node has the id `dst`, else merged
            node has a new id which is returned.
        extra_arguments : sequence, optional
            The sequence of extra positional arguments passed to
            `weight_func`.
        extra_keywords : dictionary, optional
            The dict of keyword arguments passed to the `weight_func`.

        Returns
        -------
        id : int
            The id of the new node.

        Notes
        -----
        If `in_place` is `False` the resulting node has a new id, rather than
        `dst`.
        """
        src_nbrs = set(self.neighbors(src))
        dst_nbrs = set(self.neighbors(dst))
        neighbors = (src_nbrs | dst_nbrs) - set([src, dst])

        if in_place:
            new = dst
        else:
            new = self.next_id()
            self.add_node(new)

        #对于新的edge的weight更新是不重要的，因为我的heap时在外面重新处理的edge
        for neighbor in neighbors:

            self.add_edge(neighbor, new)


        self.node[new]['mergeList'] = (self.node[src]['mergeList'] +
                                    self.node[dst]['mergeList'])
        #histogram是一个np.array(),所以可以直接相加，上面label是一个list，所以是append
        self.node[new]['hist']=(self.node[src]['hist']+
                                     self.node[dst]['hist'])
        self.node[new]['freqHist']=(self.node[src]['freqHist']+
                                     self.node[dst]['freqHist'])
        self.node[new]['pixelCount']=(self.node[src]['pixelCount']+
                                      self.node[dst]['pixelCount'])
        #remove src之后所有和src有关了的edge也都消失了
        self.remove_node(src)

        if not in_place:
            self.remove_node(dst)

        return new

    #每个RAG node都有一个histogram的property和一个pixelcount property,这个写到class外面好了
    def add_node(self, n, attr_dict=None, **attr):
        """Add node `n` while updating the maximum node id.

        .. seealso:: :func:`networkx.Graph.add_node`."""
        super(RAG, self).add_node(n, attr_dict, **attr)
        self.max_id = max(n, self.max_id)

    def add_edge(self, u, v, attr_dict=None, **attr):
        """Add an edge between `u` and `v` while updating max node id.

        .. seealso:: :func:`networkx.Graph.add_edge`."""
        super(RAG, self).add_edge(u, v, attr_dict, **attr)
        self.max_id = max(u, v, self.max_id)

    def copy(self):
        """Copy the graph with its max node id.

        .. seealso:: :func:`networkx.Graph.copy`."""
        g = super(RAG, self).copy()
        g.max_id = self.max_id
        return g

    def next_id(self):
        """Returns the `id` for the new node to be inserted.

        The current implementation returns one more than the maximum `id`.

        Returns
        -------
        id : int
            The `id` of the new node to be inserted.
        """
        return self.max_id + 1