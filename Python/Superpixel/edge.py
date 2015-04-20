# -*- coding: utf-8 -*-

"""
定义edge
"""
class myEdge(object):
    def __init__(self, src, dest,weight):
        self.src = src
        self.dest = dest
        self.weight=weight


    """
    用来表示edge的输出表示方法 0—>2: 3
    """
    def __repr__(self):
        return '{} -> {} : {}'.format(self.src,
                                  self.dest,
                                  self.weight)

    def __cmp__(self, other):
        if hasattr(other, 'weight'):
            return cmp(self.weight,other.weight )

edgeList=[myEdge(0,1,2) for i in range(2)]
edgeList.append(myEdge(1,3,4))
edgeList.append(myEdge(4,3,0))
edgeList=sorted(edgeList)
