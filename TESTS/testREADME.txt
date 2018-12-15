TESTING PROTOCOLS FOR NICKY AND KYLES'S SUPRISINGLY APATHETIC NETWORK OPTIMIZER (NAKSANO)
-Nicky Nocerino and Kyle Burge

Required Libraries: Graphviz

IN order to run the test: in TESTS, ./tests.sh
This will run all tests and populate the Folder with the tools output for each input found in TESTS, as well as Graphviz graphs for each iteration
a log of the tools running can be found in TESTS/testlog.txt

For this section of Test each round of testing was done with 2 different network examples, one of small size and complexity that begins complete (simtest), and one with medium size and complexity that begins incomplete (compgraph). The budget allowed for each round of testing was different in order to test each of the different stages of optimiztion


TEST 1 & 2:
	This test allows the optimizer to use a budget proportional to the the network size that would allow it to become complete and begin optimizations, but would be insufficient to fill(connect every node with every other node) or fully upgrade(make each wire the optimal type regardless of cost) the network.

Results:
	Both Tests were succefull in acheiving a complete state (each node can reach each other node) and continuing to upgrade, stopping before becoming filled and fully upgraded. The descriptions of the generaed networks can be found in compgraphout.xml and simtestout.xml, as can graphviz generated representations of the initial and optimized graphs. An interesting trend discoveredin this test was that upgrades were often heavily favored over the addition of connections in the first several mutation. We theorize that this is due to bandwith acting as the limiting factor in thes networks. This theory is supported by testing done in the simulator, but further testing would be required to confirm.

TESTS 3 & 4:
	This test involved the tools handling of budgets to small even to complete(each node can reach each other node) the graph. each network was given a budget proportional to its sie to garuntee it would be incapable of being complleted by the tool. when a network is incapable of being completed it should throw an error.

Results:
	In Test 3 Because the network was already complete, no resources were allocated into completeing it and so it moved forward in to the optimiztion phase, budget untouched. This is ideal action for this situation, and while it was not able to afford any upgrades, it did not throw the erorr that would have been expected.
	In Test 4, as the network was not initially complete and it had insufficient budgetto complete it, it properly threw the "insufficient funds to provide a complete network" error.

The descriptions of the generaed networks can be found in compgraphout2.xml and simtestout2.xml, as can graphviz generated representations of the initial and optimized graphs.

TEST 5 & 6
	This test was to test the ability of the tool to stop optimizing after filling and completely upgrading a network in the case of excessive budget. Both networks were provided with a budgt far larger than the amount need to fill and completely upgrade them.

Results:
	In the case of both Networks the tool was capable of recognizing their status as filled and completely upgraded immediately and cease any further optimiztions, returning to the user the remaining budget.

	The descriptions of the generaed networks can be found in compgraphout3.xml and simtestout3.xml, as can graphviz generated representations of the initial and optimized graphs.

