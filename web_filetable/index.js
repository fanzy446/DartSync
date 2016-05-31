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
	var entries = str.split("|");
	entries.pop();
	var maxLevel = 1;
	for(var i = 0; i<entries.length; i++){
		entries[i] = entries[i].split(" ");
		var tmpObj = {};
		tmpObj.fullPath = entries[i][0];
		tmpObj.size = entries[i][1];
		tmpObj.timestamp = entries[i][2];
		tmpObj.paths = tmpObj.fullPath.split("/");
		tmpObj.level = tmpObj.paths.length;
		if(tmpObj.level > maxLevel){
			maxLevel = tmpObj.level;
		}
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

		htmlStr +="<div style='margin-left:" + pointer[0].level*50 + "px;margin-right:20px'>"+ icon + pointer[0].name +"</div>";
		prev = pointer[0];
		if(pointer[0].children){
			pointer = pointer[0].children;
		}else{
			htmlStr += "<div style='float:right;position: relative;top:-24px;'><span>size:"+ prev.size +"</span> | <span>timestamp:"+ prev.timestamp +"</span></div>"
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
	htmlStr += "<div><span class='glyphicon glyphicon-refresh' aria-hidden='true'></span> &nbsp Last Update: "+ new Date() +"</div>";
	return htmlStr;
}