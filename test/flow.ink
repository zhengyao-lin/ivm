import graph

loc g1 = graph.Graph(4)

g1.con(0, 1, 4)
g1.con(0, 2, 3)
g1.con(1, 2, 2)
g1.con(1, 3, 4)
g1.con(2, 3, 2)

print(g1.max_flow())

loc g2 = graph.Graph(7)

g2.con(0, 1, 28)
g2.con(0, 2, 7)
g2.con(0, 3, 19)
g2.con(1, 4, 15)
g2.con(1, 2, 6)
g2.con(2, 3, 12)
g2.con(3, 1, 7)
g2.con(3, 4, 14)
g2.con(3, 6, 36)
g2.con(4, 5, 7)
g2.con(4, 6, 23)
g2.con(5, 2, 10)
g2.con(5, 6, 18)

print(g2.max_flow())

// -> "num: 6"
// -> "num: 46"
