!/bin/bash
cd ..
make
echo "=======TEST 1========"
echo "=======TEST 1========" > TESTS/testlog.txt
./tool TESTS/simtest.xml TESTS/simtestout.xml >> TESTS/testlog.txt
#uses graphviz to generate drawings in PNG format
dot -Tpng Input_graph.dot -o TESTS/simtestIn.png
dot -Tpng Output_graph.dot -o TESTS/simtestOut.png

echo "=======TEST 2========" >> TESTS/testlog.txt
echo "=======TEST 2========"
./tool TESTS/compgraph.xml TESTS/compgraphout.xml >> TESTS/testlog.txt
#uses graphviz to generate drawings in PNG format
dot -Tpng Input_graph.dot -o TESTS/compgraphIn.png
dot -Tpng Output_graph.dot -o TESTS/compgraphOut.png

# test with tiny budget
echo "=======TEST 3========"
echo "=======TEST 3========" >> TESTS/testlog.txt
./tool TESTS/simtest2.xml TESTS/simtest2out.xml >> TESTS/testlog.txt
#uses graphviz to generate drawings in PNG format
dot -Tpng Input_graph.dot -o TESTS/simtestIn2.png
dot -Tpng Output_graph.dot -o TESTS/simtestOut2.png

echo "=======TEST 4========"
echo "=======TEST 4========" >> TESTS/testlog.txt
./tool TESTS/compgraph2.xml TESTS/compgraph2out.xml >> TESTS/testlog.txt
#uses graphviz to generate drawings in PNG format
dot -Tpng Input_graph.dot -o TESTS/compgraphIn2.png
dot -Tpng Output_graph.dot -o TESTS/compgraphOut2.png

#test with exuberant budget
echo "=======TEST 5========"
echo "=======TEST 5========" >> TESTS/testlog.txt
./tool TESTS/simtest3.xml TESTS/simtest3out.xml >> TESTS/testlog.txt
#uses graphviz to generate drawings in PNG format
dot -Tpng Input_graph.dot -o TESTS/simtest3In.png
dot -Tpng Output_graph.dot -o TESTS/simtest3Out.png

echo "=======TEST 6========"
echo "=======TEST 6========" >> TESTS/testlog.txt
./tool TESTS/compgraph3.xml TESTS/compgraph3out.xml >> TESTS/testlog.txt
#uses graphviz to generate drawings in PNG format
dot -Tpng Input_graph.dot -o TESTS/compgraphIn3.png
dot -Tpng Output_graph.dot -o TESTS/compgraphOut3.png