// Node.js is licensed for use as follows:

// """
// Copyright Node.js contributors. All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
// """

/*---------Socket.io license----------

(The MIT License)

Copyright (c) 2014-2015 Automattic <dev@cloudup.com>

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
'Software'), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);

var net = require('net');

if (process.argv.length <= 3) {
    console.log("Please Provide Server's IP and Port");
    process.exit(-1);
}
 
var param_host = process.argv[2];
var param_port = process.argv[3];

var HOST = param_host;
var PORT = param_port;

app.get('/', function(req, res){
  res.sendfile('index.html');
});

io.on('connection', function(socket){
  console.log('a user connected');
  socketToTracker();
});

http.listen(3000, function(){
  console.log('listening on *:3000');
});


function socketToTracker(){
	var client = new net.Socket();
	client.connect(PORT, HOST, function() {
	    console.log('CONNECTED TO: ' + HOST + ':' + PORT);
	});

	client.on('data', function(data) {
	    console.log('DATA: ' + data.toString());
	    var sum = 0;
	    var jsonArr = data.toJSON();
	    for(var i = 0; i<jsonArr.length; i++){
	    	sum += jsonArr[i];
	    }
	    console.log(sum);
	    if(sum > 0){
	  		io.emit('on html', {data:parseFileData(data.toString())});
	  	}
	    //client.destroy();

	});

	client.on('close', function() {
	    console.log('Connection closed');
	});
}

function parseFileData(str){
	var htmlStr = "";
	htmlStr +='<div id="table_head"><span style="display:inline-block;width:35%; ">Path</span><span style="margin-left:150px;display:inline-block;width:10%;">File size</span> <span style="margin-left:100px;display:inline-block;width:10%;">Timestamp</span></div>';
	htmlStr += "<div><span class='glyphicon glyphicon-refresh' aria-hidden='true'></span> &nbsp Last Update: "+ new Date() +"</div>";
	var entries = str.split("|");
	entries.pop();
	for(var i = 0; i<entries.length; i++){
		entries[i] = entries[i].split(" ");
		var tmpObj = {};
		tmpObj.fullPath = entries[i][0];
		tmpObj.size = entries[i][1];
		tmpObj.timestamp = entries[i][2];
		tmpObj.paths = tmpObj.fullPath.split("/");
		tmpObj.level = tmpObj.paths.length;
		entries[i] = tmpObj;
	}

	var tree = {children:[]};
	for(var j =0; j<entries.length; j++){
		var pointer = tree;
		for(var k = 0; k<entries[j]["level"]; k++){
			var exist = -1;
			if(pointer.children){
				for(var t = 0; t<pointer["children"].length; t++){
					if(pointer["children"][t]["name"] == entries[j]["paths"][k]){
						exist = t;
						break;
					}
				}
			}
			if(exist > -1){
				pointer = pointer["children"][exist];
			}else{
				tmpObj = {};
				if(k != entries[j]["level"] - 1){
					tmpObj.children = [];
				}
				tmpObj.type = "folder";
				tmpObj.name = entries[j]["paths"][k];
				tmpObj.level = k;
				tmpObj.parent = pointer;
				if(!pointer.children){
					pointer.children = [];
				}
				pointer.children.push(tmpObj);
				pointer = tmpObj;
			}

			if(k == entries[j]["level"] - 1){
				console.log(tmpObj.name, entries[j]["size"]);
				
				pointer.size = entries[j]["size"];
				if(pointer.size != -1){
					tmpObj.type = "file";
				}else{
					tmpObj.type = "folder";
				}
				pointer.timestamp = entries[j]["timestamp"];
			}
		}
	}


	//console.log(tree);
	var prev;
	var flagEnd = false;
	var pointer = tree.children;
	while(pointer!=null&&flagEnd==false){
		var icon = "";
		if(pointer[0].type == 'folder'){
			icon = "<span class='glyphicon glyphicon-folder-open' aria-hidden='true'></span>&nbsp/";
		}else{
			icon = "<span class='glyphicon glyphicon-file' aria-hidden='true'></span>&nbsp";
		}
		htmlStr +="<div class='single_line'><span style='display:inline-block;width:35%; padding-left:" + pointer[0].level*50 + "px;'>"+ icon + pointer[0].name + "</span>";
		//console.log(pointer[0].name, pointer[0].level);
		prev = pointer[0];
		if(pointer[0].children){
			pointer = pointer[0].children;
			htmlStr += "</div>";
		}else{
			htmlStr += "<span style='margin-left:150px;display:inline-block;width:10%;'>"+ prev.size +"</span> <span style='margin-left:100px;display:inline-block;width:10%;'>"+ prev.timestamp +"</span></div>"
			//console.log(prev.size, prev.timestamp);
			while(pointer.length<=1){
				prev = prev.parent;
				pointer.shift();
				if(prev){
					pointer = prev.children;
				}else{
					flagEnd = true;
					break;
				}
			}
			pointer.shift();
		}
	}
	
	//document.body.innerHTML += htmlStr;
	return htmlStr
}