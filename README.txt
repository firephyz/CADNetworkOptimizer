NICKY AND KYLES SUPER AWESOME NETWORK OPTIMIZER (NAKSANO)

NAKSANO is a network optimiztion tool that takes in a XML description of a network, an budget of upgrades, and the users preferences for prioritizing latency, error rate, and throughput, in order to output a XML description of the optimally upgraded grpah for the defined budget. I acomplishes this by utilizing a simulator to determine the average latency, error rate, and throughput of the network, then determines bust possible upgrades and creates mutations of the network, simulating each one, and adding the most benificial (defined by the users preferences) mutation to the network. It then repeats this process until either budget is used up, or the network is completely filled(all nodes are adirectly conneted to all other nodes) and fully upgraded(every wire is the optimal type, regardless of price).

USE:

in /CADNetWorkOptimizer/
1. make
2. ./tool <input_file.xml> <output_file.xml>

Run time will vary, but increases with size of the network.

Required Libraries:

-Libxml2
-Graphviz

TESTS:

	Within the TESTS directory can be found tests.sh which is a bash script to run several tests of NAKSANO. Further details can be found in TESTS/testsREADME.txt

DATA:

	Within the DATA folder can be found data on our simulator, as well as the results of runs on many networks manipulating Bandwith, sendRate, recieveRate, and routeRate in order to understand theses statisic's effets on on latency, error rate, and throughput. Code, data and graphs regarding this can be found here.

GITHUB:

https://github.com/firephyz/CADNetworkOptimizer
