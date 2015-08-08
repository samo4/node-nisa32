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

instrument11.open(function (error) {
  if ( error ) {
    console.log('failed to open: '+ error);
  } else {
    console.log('dcl');
	  instrument11.dcl(function(err, res) { 
      if ( error ) {
        console.log('failed to open: '+ error);
      } else {
        console.log(res);
        instrument11.query("J0", function(err, res) {
            console.log(err);
            console.log(res);
        });
      }
    });
  }
});

/*
var VisaPort = nisa.VisaPort;
var instrument12 = new VisaPort("GPIB0::12::INSTR", {
  bufferSize: 256
});

instrument12.open(function (error) {
  if ( error ) {
    console.log('failed to open: '+ error);
  } else {
    console.log('open');
	instrument12.on('data', function(data) {
      console.log('this shall not work for a long long time....:-) data received: ' + data);
    });
  }
});

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