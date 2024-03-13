import numpy.random

fileName = "./test.dot"
nodeNum = 2000
rate = 0.2

file = open(fileName, "w")
file.write("graph G {\n")
for i in range(nodeNum):
    file.write("v" + str(i) + "\n")

for i in range(nodeNum):
    for j in range(nodeNum):
        if i == j:
            continue
        rd = numpy.random.random()
        if rd < rate:
            file.write("v" + str(i) + "--" + "v" + str(j) + "\n")

file.write("};\n")
file.close()