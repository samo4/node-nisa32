/************************************************************

 NODE.JS INTERACTIVE IO
 
This is WEB interface for visa32test.js.

Author : 7M4MON
Date : 2015/04/24
Licence : MIT

************************************************************/


var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);

var visa32test = require('./nisa32.js');

// visa32test.Visa32TestQuery('GPIB0::12::INSTR','*IDN?');

app.get('/',function(req,res){
    res.sendFile('index.html');
});
var rcvMsg;
io.on('connection',function(socket){
    socket.on('sendmsg',function(msg){
        rcvMsg = visa32test.query(msg.addr,msg.cmd, function(err, result) {
				if (err)
					io.emit('recvmsg', err);
				else
					io.emit('recvmsg', result);
			}
		);
    });
});

http.listen(3000,function(){
    console.log('listen 3000 port');
});
