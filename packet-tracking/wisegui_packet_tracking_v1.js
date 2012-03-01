/*
+------+-----------+---------+---------+--------+
| TYPE | DIRECTION | SRC_MAC | DST_MAC | PACKET |
+------+-----------+---------+---------+--------+
 0      1           23456789  01234567  8... 

TYPE: (1 Byte) 32 for MAC Pakets, 16 for IPv6 Pakete
DIRECTION: (1 Byte) 'i' für incoming, 'o' for outgoing
SRC_MAC: (8 Byte) Source MAC Address
DST_MAC: (8 Byte) Destination MAC Address
PACKET: (max. 1280 Byte) RAW IPv6 packet, i.e. (IPv6 Header | Transport Header | Application Header / Payload ) or MAC Payload
*/

var UART_MESSAGE_TYPE_NETWORK_OUT = 115;

Packet = function(nodeUrn, type, incoming, srcMac, dstMac, payload) {
    this.nodeUrn = nodeUrn;
    this.type = type;
	this.incoming = incoming;
	this.srcMac = srcMac;
	this.dstMac = dstMac;
	this.payload = payload;
}

WiseGuiUserScript = function() {
	
	this.testbedId = null;
	this.experimentId = null;
	this.webSocket = null;
	this.outputDiv = null;
	
	this.graph = null;
	this.graphWidth = null;
	this.graphHeight = null;
	this.graphPosFactorX = null;
	this.graphPosFactorY = null;
	this.networkNodes = [];
	this.networkNodesIndices = {};
	this.networkData = {
		minX : Number.MAX_VALUE,
		maxX : Number.MIN_VALUE,
		minY : Number.MAX_VALUE,
		maxY : Number.MIN_VALUE
	};
	
	this.redraw = true;
};

WiseGuiUserScript.prototype.loadCSS = function() {
	
	this.style = document.createElement('style');
	this.style.type = 'text/css';
	
	var rules = document.createTextNode(
		'circle.node { fill: #aaa; stroke: #666; stroke-width: 1px; }\n'+
		'text.nodeurn { fill: #000; }\n'+
		'text.nodemsgs { fill: #000; }\n'
	);
	
	this.style.type = 'text/css';
	
	if(this.style.styleSheet) {
		this.style.styleSheet.cssText = rules.nodeValue;
	}
	else {
		this.style.appendChild(rules);
	}
		
	document.body.appendChild(this.style);
};

WiseGuiUserScript.prototype.unloadCSS = function() {
	document.body.removeChild(this.style);
};

WiseGuiUserScript.prototype.loadD3 = function() {
	this.d3 = document.createElement('script');
	this.d3.src = 'http://wisebed.itm.uni-luebeck.de/js/lib/d3/d3-2.8.0.js';
	document.body.appendChild(this.d3);
};

WiseGuiUserScript.prototype.unloadD3 = function() {
	document.body.removeChild(this.d3);
};

WiseGuiUserScript.prototype.start = function(env) {
	
	//console.log("start()");
	
	this.loadD3();
	this.loadCSS();
	
	this.testbedId = env.testbedId;
	this.experimentId = env.experimentId;
	this.outputDiv = env.outputDiv;
	
	this.graphContainer = $('<div class="GraphContainer container span16" style="height: 500px; overflow: auto; border: 1px solid #aaa;"></div>');
	this.outputDiv.append(this.graphContainer);
	
	//this.statsContainer = $('<div class="StatsContainer container span16" style="height: 300px; overflow: auto; border: 1px solid #aaa;"></div>');
	//this.outputDiv.append(this.statsContainer);
	
	var self = this;
	
	this.webSocket = new Wisebed.WebSocket(
		this.testbedId,
		this.experimentId,
		function() {self.onmessage(arguments[0]);},
		function() {self.onopen(arguments);},
		function() {self.onclosed(arguments);}
	);
	
	Wisebed.getWiseMLAsJSON(
		env.testbedId,
		env.experimentId,
		function(data) {self.loadData(data);},
		function() {alert(arguments);}
	);
};

WiseGuiUserScript.prototype.loadData = function(wiseml) {
	
	//console.log("loadData()");
	
	var self = this;
	$.each(wiseml.setup.node, function(index, elem) {
		self.networkNodes[index] = {
			nodeUrn : elem.id,
			pos : {
				x : elem.position.x,
				y : elem.position.y
			},
			msgs : {
				incoming : 0,
				outgoing : 0
			}
		};
		
		self.networkNodesIndices[elem.id] = index;
		
		if (elem.position.x > self.networkData.maxX) { self.networkData.maxX = elem.position.x }
		if (elem.position.x < self.networkData.minX) { self.networkData.minX = elem.position.x }
		if (elem.position.y > self.networkData.maxY) { self.networkData.maxY = elem.position.y }
		if (elem.position.y < self.networkData.minY) { self.networkData.minY = elem.position.y }
		
	});
	
	self.loadingDone();
};

WiseGuiUserScript.prototype.loadingDone = function() {
	
	//console.log("loadingDone()");
	
	this.initGraph();
	
	this.updateGraph();
	//this.printStats();
	
	var self = this;
	setInterval(function() {
		self.updateGraph();
		//self.printStats();
	}, 1000);
};

WiseGuiUserScript.prototype.printStats = function() {
	
	//console.log("printStats()");
	
	this.statsContainer.empty();
	
	var self = this;
	$.each(this.networkNodes, function(index, nodeData) {
		self.statsContainer.append(
			nodeData.nodeUrn + " | " +
			"(" + nodeData.pos.x + "," + nodeData.pos.y + ") | " +
			"(" + nodeData.msgs.incoming + "," + nodeData.msgs.outgoing + ")" +
			"<br/>"
		);
	});
};

WiseGuiUserScript.prototype.initGraph = function() {
	
	//console.log("initGraph()");
	
	var graphWidth = 930;
	var graphHeight = 500;
	
	this.graph = d3.select('.GraphContainer').append('svg')
			.attr('width', graphWidth)
			.attr('height', graphHeight);
	
	this.graphPosFactorX = graphWidth / this.networkData.maxX;
	this.graphPosFactorY = graphHeight / this.networkData.maxY;
	
	var self = this;
	var data = this.graph.selectAll('.data')
		.data(this.networkNodes, function(d) { return d.nodeUrn })
		.enter()
			.append('g')
			.attr('class', 'data')
			.attr('transform', function(d) { return 'translate('+(d.pos.x * self.graphPosFactorX)+','+(d.pos.y * self.graphPosFactorY)+')'});
	
	data.append('circle')
			.attr('class', 'node')
			.attr('r', function(d) {
					var radius = Math.sqrt(d.msgs.incoming + d.msgs.outgoing);
					return radius == 0 ? 10 : 10 + radius;
			});
			
	data.append('text')
			.attr('dy', '.35em')
			.attr('class', 'nodemsgs')
			.attr('text-anchor', 'middle')
			.text(function(d) { return d.msgs.incoming + d.msgs.outgoing; });
			
	data.append('text')
			.attr('dy', '2em')
			.attr('class', 'nodeurn')
			.attr('text-anchor', 'middle')
			.text(function(d) { return d.nodeUrn; });
};

WiseGuiUserScript.prototype.updateGraph = function() {
	
	//console.log("updateGraph()");
	
	if (this.redraw) {
	
		var data = this.graph.selectAll('.data').data(this.networkNodes);
		
		data.selectAll('circle').attr('r', function(d) {
			var radius = Math.sqrt(d.msgs.incoming + d.msgs.outgoing);
			return radius == 0 ? 10 : 10 + radius;
		});
		
		data.selectAll('text.nodemsgs').text(function(d) {
			return d.msgs.incoming + d.msgs.outgoing;}
		);
		
		this.redraw = false;
	}
};

WiseGuiUserScript.prototype.stop = function() {
	
	//console.log("stop()");
	
	this.webSocket.close();
	this.unloadD3();
	this.unloadCSS();
	this.outputDiv.empty();
	
	delete Packet;
};

WiseGuiUserScript.prototype.onmessage = function(message) {
	
	//console.log("onmessage()");
	
	var decodedMessage = atob(message.payloadBase64);
	var nodeUrn = message.sourceNodeUrn;
	var messageBytes = [];
	for(i = 0; i < decodedMessage.length; i++) {
		messageBytes.push(decodedMessage.charCodeAt(i));
	}
	var view = new Uint8Array(messageBytes);
	
	var packetType = view[0] & 0xFF;
	
	if (packetType == UART_MESSAGE_TYPE_NETWORK_OUT) {
	
	var type = (view[1] & 0xFF) == 16 ? 'ipv6' : ((view[1] & 0xFF) == 32 ? 'mac' : 'unknown');
	var incoming = String.fromCharCode(view[2] & 0xFF) == 'i' ? true : false;
	var srcMac = (view[8] & 0xFF) + (view[9] & 0xFF);
	var dstMac = (view[10] & 0xFF) + (view[11] & 0xFF);
	var payload = view.subarray(18);
	
	this.onTracingPacketReceived(new Packet(nodeUrn, type, incoming, srcMac, dstMac, payload));
  }
};

WiseGuiUserScript.prototype.onopen = function(event) {
	//console.log(event);
};

WiseGuiUserScript.prototype.onclosed = function(event) {
	//console.log(event);
};

WiseGuiUserScript.prototype.onTracingPacketReceived = function(packet) {
	
	//console.log("onTracingPacketReceived()");
	
	this.redraw = true;
	if (packet.incoming) {
		this.networkNodes[this.networkNodesIndices[packet.nodeUrn]].msgs.incoming++;
	} else {
		this.networkNodes[this.networkNodesIndices[packet.nodeUrn]].msgs.outgoing++;
	}
};
