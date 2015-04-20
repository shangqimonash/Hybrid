
# -*- coding: utf-8 -*-
class Node:
    def __init__ (self, label):
        self.label = label
    def __str__(self):
        return self.label


class UnionSet:
    def __init__(self,num):
        self.all=[Node(i) for i in range(num)]
        self.num=num
        [self.makeSet(x) for x in self.all]

    def makeSet(self,x):
        x.parent=x
        x.size=1


    """
    find 输入 index 返回 root index
    """
    def find(self,xindex):
        x=self.all[xindex]
        root=self.innerFind(x)
        return self.all.index(root)


    """
    方便递归查找
    """
    def innerFind(self,x):
        if x.parent==x:
            return x
        else:
            x.parent=self.innerFind(x.parent)
            return x.parent

    def setNodeLabel(self,index,label):
        x=self.all[index]
        root=self.innerFind(x)
        root.label=label

    def segLabel(self,index):
        rootIndex=self.find(index)
        x=self.all[rootIndex]
        return x.label

    def union(self,xIndex,yIndex):

        x=self.all[xIndex]
        y=self.all[yIndex]

        xRoot=self.innerFind(x)
        yRoot=self.innerFind(y)

        if xRoot==yRoot:
            return

        self.num-=1
        if xRoot.size>yRoot.size:
            yRoot.parent=xRoot
            xRoot.size+=yRoot.size
        else:
            xRoot.parent=yRoot
            yRoot.size+=xRoot.size

    def size(self,xIndex):
        x=self.all[xIndex]
        xRoot= self.innerFind(x)
        return xRoot.size
    def setNum(self):
        return self.num