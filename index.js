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

var nisa = require('./lib/nisa32.js');
var async = require('async');
/*nisa.list(function(err, res) { 
  if ( err ) {
    console.log('failed to open for list: '+ err);
  } else {
    console.log(res); 
  }
});*/


var VisaPort = nisa.VisaPort;
var instrument11 = new VisaPort("GPIB0::11::INSTR", {
  bufferSize: 256
});

var instrument12 = new VisaPort("GPIB0::12::INSTR", {}, true, function (err, res) {
  if ( !err ) {
    async.series ([
        function(callback) { instrument12.query("VSET1,1", callback) },
        function(callback) { setTimeout(callback, 300) }, 
        function(callback) { instrument12.query("VSET1,2", callback) },
        function(callback) { setTimeout(callback, 300) }, 
        function(callback) { instrument12.query("VSET1,3", callback) },
        function(callback) { setTimeout(callback, 300) }, 
        function(callback) { instrument12.query("VSET1,4", callback) },
        function(callback) { setTimeout(callback, 300) }, 
        function(callback) { instrument12.query("VSET1,5", callback) },
        function(callback) { instrument12.readStatusByte(callback); },
      ], function(err, res) {
         if (err) { 
           console.log('ERROR12');
           console.log(err);
         } else {
           console.log('DONE12');
           console.log(res);
         }   
      });
  }
});

async.series ([
  function(callback) { instrument11.open(callback); },
  function(callback) { instrument11.ibsre(true, callback); },
  function(callback) { instrument11.clear(callback); },
  function(callback) { instrument11.write(String.fromCharCode(13), callback) },
  function(callback) { setTimeout(callback, 500) }, 
  function(callback) { instrument11.write("D4 1X", callback) },
  function(callback) { setTimeout(callback, 500) }, 
  function(callback) { instrument11.write("D4 2X", callback) },
  function(callback) { setTimeout(callback, 500) }, 
  function(callback) { instrument11.write("D4 3X", callback) },
  function(callback) { setTimeout(callback, 500) }, 
  function(callback) { instrument11.write("D4 4X", callback) },
  function(callback) { setTimeout(callback, 500) }, 
  function(callback) { instrument11.write("D0X", callback) },
  function(callback) { instrument11.readStatusByte(callback); },
  
], function(err, res) {
   if (err) { 
     console.log('ERROR');
     console.log(err);
   } else {
     console.log('DONE');
     console.log(res);
   }   
});

/*

app.get('/',function(req,res){
    res.sendFile(__dirname + '/index.html');
});
var rcvMsg;
io.on('connection',function(socket){
    socket.on('sendmsg',function(msg){
        rcvMsg = instrument12.query(msg.cmd, function(err, result) {
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
*/