# -*- coding: utf-8 -*-



def change(a,x,y):
    tmp=a[x]
    a[x]=a[y]
    a[y]=tmp
    c=add(x,y)
    newList=[32,23,5]
    return newList

def add(x,t):
    return x+t
a=[3,4,5]
d=change(a,0,1)
print a
print d