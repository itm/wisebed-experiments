WISEBED experiments
==============
This project contains a set of configurations for [WISEBED](http://wisebed.eu) experiments. 

How to use them
-------------
Copy the link to a .json file and go to the [WISEBED testbed GUI](http://wisebed.eu/site/testbed). 
Select a testbed, log in, create a reservation for a set of nodes, select this reservation, and go to
the "Flash"-dialog. There, click on the "Load" button and paste the URL into the textfield and click ok.

This configuration will be loaded and can afterwards be used to flash the nodes.

How does it work
-------------
A configuration is a simple file that specifies which sensor nodes should be flashed with a certain binary. 
A configuration is a simple JSON-file containing so-called "configuration"-entries.

Each entry specifies which nodes should receive which binary. Each configuration contains two links, one that 
links to a JSON file with an array of nodes and one that links to the binary.