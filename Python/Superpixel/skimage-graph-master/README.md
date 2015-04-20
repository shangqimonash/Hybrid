skimage-graph
=============



# Results
## Custom class

**Memory**
```
Line #    Mem usage    Increment   Line Contents
================================================
    13   18.184 MiB    0.000 MiB   @profile
    14                             def test():
    15  496.137 MiB  477.953 MiB       arr = np.load("../data/watershed.npy")
    16  496.137 MiB    0.000 MiB       t = time.time()
    17  507.371 MiB   11.234 MiB       g = graph.construct_rag(arr)
    18                             
    19  507.383 MiB    0.012 MiB       print "RAG construction took %f secs " % (time.time() - t)
    20                             
    21                                 # print g.max_size
    22  507.383 MiB    0.000 MiB       t = time.time()
    23  506.543 MiB   -0.840 MiB       g.random_merge(10)
    24  506.543 MiB    0.000 MiB       g.display()
    25                                 # print g.max_size
    26  506.543 MiB    0.000 MiB       print "Merging took %f secs " % (time.time() - 
```


**Time**
```
RAG construction took 40.705932 secs 
Merging took 0.991784 secs
```
**Time - Construction**
```
Line #      Hits         Time  Per Hit   % Time  Line Contents
==============================================================
    24                                               @profile
    25                                               def make_edge(self, i, j, wt):
    26  72157546     40339563      0.6     19.9          try:
    27  72157546     56802957      0.8     28.0              self.rows[i][j]
    28     73778        92866      1.3      0.0          except KeyError:
    29     73778        72716      1.0      0.0              self.edge_count += 1
    30                                           
    31  72157546     52556299      0.7     25.9          self.rows[i][j] = wt
    32  72157546     53126454      0.7     26.2          self.rows[j][i] = wt
```
**Time - Merging**
```
Line #      Hits         Time  Per Hit   % Time  Line Contents
==============================================================
    41                                               @profile
    42                                               def merge(self, i, j):
    43                                           
    44     20378        48653      2.4      3.5          if not self.has_edge(i, j):
    45                                                       raise ValueError('Cant merge non adjacent nodes')
    46                                                       
    47                                                   
    48                                                   # print "before ",self.order()
    49     94147       100716      1.1      7.3          for x in self.neighbors(i):
    50     73769        56567      0.8      4.1              if x == j:
    51     20378        14158      0.7      1.0                  continue
    52     53391        94329      1.8      6.8              w1 = self.get_weight(x, i)
    53     53391        38553      0.7      2.8              w2 = -1
    54     53391       181510      3.4     13.1              if self.has_edge(x, j):
    55     19352        29334      1.5      2.1                  w2 = self.get_weight(x,j)
    56                                           
    57     53391        59662      1.1      4.3              w = max(w1, w2)
    58                                           
    59     53391       648168     12.1     46.8              self.make_edge(x, j, w)
    60                                           
    61     20378       113651      5.6      8.2          self.remove_node(i)

```    

## Networx Graph Class

**Memory**
```
Line #    Mem usage    Increment   Line Contents
================================================
    12   23.023 MiB    0.000 MiB   @profile
    13                             def test():
    14  500.918 MiB  477.895 MiB       arr = np.load("../data/watershed.npy")
    15  500.918 MiB    0.000 MiB       t = time.time()
    16  530.703 MiB   29.785 MiB       g = graph.construct_rag(arr)
    17                                 
    18                                 
    19  530.719 MiB    0.016 MiB       print "RAG construction took %f secs " % (time.time() - t)
    20                             
    21  530.719 MiB    0.000 MiB       t = time.time()
    22  517.906 MiB  -12.812 MiB       g.random_merge(10)
    23                                 #g.display()
    24  517.906 MiB    0.000 MiB       print "Merging took %f secs " % (time.time() - t)

```

**Time**
```
RAG construction took 117.332280 secs 
Merging took 1.238992 secs
```
**Time - Construction**
```
Function to add edge is provided by networx.
```
**Time - Merging**
```
Line #      Hits         Time  Per Hit   % Time  Line Contents
==============================================================
    26                                               @profile
    27                                               def merge(self, i, j):
    28                                           
    29     20378        45880      2.3      4.2          if not self.has_edge(i, j):
    30                                                       raise ValueError('Cant merge non adjacent nodes')
    31                                           
    32                                                   # print "before ",self.order()
    33     94147       115543      1.2     10.5          for x in self.neighbors(i):
    34     73769        58640      0.8      5.3              if x == j:
    35     20378        14190      0.7      1.3                  continue
    36     53391       139198      2.6     12.6              w1 = self.get_edge_data(x, i)['weight']
    37     53391        39027      0.7      3.5              w2 = -1
    38     53391       103894      1.9      9.4              if self.has_edge(x, j):
    39     19352        42703      2.2      3.9                  w2 = self.get_edge_data(x, j)['weight']
    40                                           
    41     53391        57866      1.1      5.2              w = max(w1, w2)
    42                                           
    43     53391       313574      5.9     28.4              self.add_edge(x, j, weight=w)
    44                                           
    45     20378       172383      8.5     15.6          self.remove_node(i)

```

## LIL Graph Class

**Memory**
```
Line #    Mem usage    Increment   Line Contents
================================================
    12   18.223 MiB    0.000 MiB   @profile
    13                             def test():
    14  496.133 MiB  477.910 MiB       arr = np.load("../data/watershed.npy")
    15  496.133 MiB    0.000 MiB       t = time.time()
    16  504.375 MiB    8.242 MiB       g = graph.construct_rag(arr)
    17                                 
    18                                 
    19  504.383 MiB    0.008 MiB       print "RAG construction took %f secs " % (time.time() - t)
    20                             
    21  504.383 MiB    0.000 MiB       t = time.time()
    22  504.910 MiB    0.527 MiB       g.random_merge(10)
    23  504.965 MiB    0.055 MiB       g.display()
    24  504.965 MiB    0.000 MiB       print "Merging took %f secs " % (time.time() - t)
```

**Time**
```
RAG construction took 689.007435 secs 
Merging took 201.393969 secs
```
**Time - Construction**
```
   Ordered by: internal time

   ncalls  tottime  percall  cumtime  percall filename:lineno(function)
 72104155  275.539    0.000  640.501    0.000 rag_lil.pyx:18(add_edge)
144208310  274.654    0.000  274.654    0.000 {method 'searchsorted' of 'numpy.ndarray' objects}
        1  164.755  164.755  999.104  999.104 {rag_lil.construct_rag_3d_lil}
144208312   90.060    0.000   90.060    0.000 stringsource:317(__cinit__)
144208310   84.936    0.000  359.589    0.000 fromnumeric.py:952(searchsorted)
144208311   66.758    0.000  156.817    0.000 stringsource:613(memoryview_cwrapper)
144208312   21.694    0.000   21.694    0.000 stringsource:339(__dealloc__)
144208311   15.142    0.000   15.142    0.000 stringsource:619(memoryview_check)
   158956    3.893    0.000    5.373    0.000 function_base.py:3305(insert)
   476868    0.726    0.000    0.726    0.000 {numpy.core.multiarray.array}
   199734    0.272    0.000    0.272    0.000 {numpy.core.multiarray.empty}
   158956    0.179    0.000    0.424    0.000 numeric.py:392(asarray)
   158956    0.163    0.000    0.163    0.000 numeric.py:1299(rollaxis)
        1    0.137    0.137    0.137    0.137 {method 'reduce' of 'numpy.ufunc' objects}
   158956    0.127    0.000    0.127    0.000 {isinstance}
   158956    0.051    0.000    0.051    0.000 {method 'item' of 'numpy.ndarray' objects}
        1    0.019    0.019    0.057    0.057 graph_lil.py:8(__init__)
        1    0.000    0.000    0.000    0.000 {range}
        1    0.000    0.000    0.000    0.000 {numpy.core.multiarray.zeros}
        1    0.000    0.000    0.137    0.137 fromnumeric.py:2048(amax)
        1    0.000    0.000    0.137    0.137 _methods.py:15(_amax)
        1    0.000    0.000  999.104  999.104 graph_lil.py:59(construct_rag)
        1    0.000    0.000    0.000    0.000 stringsource:957(memoryview_fromslice)
        1    0.000    0.000  999.104  999.104 <string>:1(<module>)
        1    0.000    0.000    0.000    0.000 stringsource:933(__dealloc__)
        1    0.000    0.000    0.000    0.000 stringsource:508(__get__)
        1    0.000    0.000    0.000    0.000 stringsource:468(__getbuffer__)
        1    0.000    0.000    0.000    0.000 {method 'disable' of '_lsprof.Profiler' objects}

```

**Time - Merging**
```
   Ordered by: internal time

   ncalls  tottime  percall  cumtime  percall filename:lineno(function)
    20378  117.102    0.006  252.734    0.012 rag_lil.pyx:47(merge_node)
 56129898   98.659    0.000   98.659    0.000 {method 'searchsorted' of 'numpy.ndarray' objects}
 56129898   32.130    0.000  130.789    0.000 fromnumeric.py:952(searchsorted)
   107196    1.997    0.000    2.734    0.000 function_base.py:3112(delete)
    68494    1.339    0.000    1.837    0.000 function_base.py:3305(insert)
        1    0.605    0.605  254.349  254.349 graph_lil.py:48(random_merge)
   419874    0.573    0.000    0.573    0.000 {numpy.core.multiarray.array}
   161011    0.427    0.000    0.453    0.000 random.py:173(randrange)
   161011    0.263    0.000    0.716    0.000 random.py:236(randint)
   282886    0.214    0.000    0.600    0.000 numeric.py:392(asarray)
   175690    0.205    0.000    0.205    0.000 {numpy.core.multiarray.empty}
   163024    0.162    0.000    0.162    0.000 stringsource:317(__cinit__)
   282886    0.134    0.000    0.134    0.000 {isinstance}
    20378    0.101    0.000  252.942    0.012 {rag_lil.merge_node_py}
    40756    0.085    0.000    0.108    0.000 stringsource:957(memoryview_fromslice)
   122268    0.068    0.000    0.213    0.000 stringsource:613(memoryview_cwrapper)
    68494    0.065    0.000    0.065    0.000 numeric.py:1299(rollaxis)
    20378    0.045    0.000    0.052    0.000 random.py:271(choice)
   175690    0.044    0.000    0.044    0.000 {method 'item' of 'numpy.ndarray' objects}
    20378    0.033    0.000  252.975    0.012 graph_lil.py:31(merge)
   163024    0.033    0.000    0.033    0.000 stringsource:339(__dealloc__)
   181389    0.029    0.000    0.029    0.000 {method 'random' of '_random.Random' objects}
        1    0.023    0.023  254.371  254.371 <string>:1(<module>)
   122268    0.015    0.000    0.015    0.000 stringsource:619(memoryview_check)
    40756    0.006    0.000    0.006    0.000 stringsource:508(__get__)
    40756    0.006    0.000    0.006    0.000 stringsource:468(__getbuffer__)
    40756    0.005    0.000    0.005    0.000 stringsource:933(__dealloc__)
    20378    0.005    0.000    0.005    0.000 {len}
        1    0.000    0.000    0.000    0.000 {method 'disable' of '_lsprof.Profiler' objects
```

## CSR Graph Class
**Memory**
```
Line #    Mem usage    Increment   Line Contents
================================================
    13   20.031 MiB    0.000 MiB   @profile
    14                             def test():
    15  498.074 MiB  478.043 MiB       arr = np.load("../data/watershed.npy")
    16  498.074 MiB    0.000 MiB       t = time.time()
    17  500.281 MiB    2.207 MiB       g = graph.construct_rag(arr)
    18                             
    19  500.293 MiB    0.012 MiB       print "RAG construction took %f secs " % (time.time() - t)
    20                             
    21                                 #print g.max_size
    22  500.293 MiB    0.000 MiB       t = time.time()
    23  825.211 MiB  324.918 MiB       g.random_merge(10)
    24                                 #print g.max_size
    25  825.211 MiB    0.000 MiB       print "Merging took %f secs " % (time.time() - t)

```

**Time**
```
RAG construction took 5.593051 secs 
Merging took 9.319996 secs
```

**Time - Construction**
```
Done using DOK
```
**Time - Merging**
```
   Ordered by: internal time

   ncalls  tottime  percall  cumtime  percall filename:lineno(function)
    20378    9.164    0.000   10.663    0.001 rag_csr.pyx:17(merge)
    20378    0.515    0.000   11.658    0.001 graph_csr.py:69(merge)
        8    0.451    0.056    0.452    0.057 graph_csr.py:51(double)
   288383    0.387    0.000    0.415    0.000 random.py:173(randrange)
    20378    0.353    0.000    0.353    0.000 {method 'sort' of 'numpy.ndarray' objects}
    20378    0.319    0.000    0.823    0.000 arraysetops.py:93(unique)
        1    0.318    0.318   12.573   12.573 graph_csr.py:79(random_merge)
    40756    0.265    0.000    0.270    0.000 {numpy.core.multiarray.concatenate}
   288383    0.141    0.000    0.556    0.000 random.py:236(randint)
   142646    0.134    0.000    0.134    0.000 stringsource:317(__cinit__)
    20378    0.069    0.000    0.069    0.000 {numpy.core.multiarray.copyto}
   101890    0.067    0.000    0.193    0.000 stringsource:613(memoryview_cwrapper)
    20378    0.055    0.000    0.055    0.000 {numpy.core.multiarray.empty_like}
    20378    0.051    0.000    0.175    0.000 numeric.py:78(zeros_like)
    20378    0.049    0.000    1.028    0.000 arraysetops.py:379(union1d)
    40756    0.041    0.000    0.054    0.000 stringsource:957(memoryview_fromslice)
    20378    0.037    0.000    0.037    0.000 {method 'flatten' of 'numpy.ndarray' objects}
    20378    0.035    0.000    0.041    0.000 random.py:271(choice)
        1    0.030    0.030   12.603   12.603 <string>:1(<module>)
   308761    0.030    0.000    0.030    0.000 {method 'random' of '_random.Random' objects}
   142646    0.029    0.000    0.029    0.000 stringsource:339(__dealloc__)
    20378    0.028    0.000   10.691    0.001 {rag_csr.merge}
   101890    0.014    0.000    0.014    0.000 stringsource:619(memoryview_check)
    40756    0.005    0.000    0.005    0.000 stringsource:933(__dealloc__)
    40756    0.005    0.000    0.005    0.000 stringsource:468(__getbuffer__)
    40756    0.004    0.000    0.004    0.000 stringsource:508(__get__)
    20378    0.004    0.000    0.004    0.000 {len}
       16    0.001    0.000    0.001    0.000 {numpy.core.multiarray.zeros}
        1    0.000    0.000    0.000    0.000 {method 'disable' of '_lsprof.Profiler' objects}

```