iSerAerial configuration for WISEBED experiments
==============
This is a configuration for a [WISEBED](http://wisebed.eu) experiment. 

What does this configuration provide
-------------
It provides binaries for the 
[iSerAerial protocol](https://github.com/itm/netty-handlerstack/wiki/ISerAerial-Protocol-Decoder-Encoder).

How does it work
-------------
A configuration is a simple file that specifies which sensor nodes should be flashed with a certain binary.

A configuration is a simple JSON-file containing so-called "configuration"-entries. Each entry specifies which 
nodes should receive which binary. Each configuration contains two links, one that links to a JSON file with an
array of nodes and one that links to the binary.


