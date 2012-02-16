WISEBED experiments
==============
This project contains a set of configurations for [WISEBED](http://wisebed.eu) experiments. 

How does it work
-------------
A configuration is a simple file that specifies which sensor nodes should be flashed with a certain binary. A configuration is a simple JSON-file containing so-called "configuration"-entries.

Each entry specifies which nodes should receive which binary. Each configuration contains two links, one that links to a JSON file with an array of nodes and one that links to the binary.