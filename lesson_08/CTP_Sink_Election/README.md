The CTP (Collection Tree Protocol) creates a network in which every source node sends its information to a node closer to the sink until the message gets to the sink.

This is an "one way communication". The communication occurs from the sources to the sink. Never the other way round (Sink --> Sources).


This project focus on the election of the sink node. By default, when the simulation is started, the node with ID 1 will be the sink.

After some time, the sink node is removed. Thus, it is necessary to start an election to chose the new sink node. This process is repeated until there's only one node left. Always the node with higher ID is chosen as sink. This metric could be changed as the user wishes (e.g., link quality, battery of the node...).

About the election, it works like a Bully algorithm. The main difference is that in my approach, the nodes flood the network with the elected id (what is not so good, to be honest), instead of being quiet when their ID is not selected.

This was implemented intending to collect metrics of the overhead of chosing a new sink everytime it's necessary using a naive approach.
