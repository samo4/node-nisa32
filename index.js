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

/*
var instrument12 = new VisaPort("GPIB0::12::INSTR", {}, true, function (err, res) {
  if ( !err ) {
    instrument12.query("ID?", function(err, result) {
					 console.log(result);
			});
  }
});*/

instrument11.open(function (error) {
  if ( !error ) {
    console.log('now dcl');
    instrument11.ibsre(true,function(err, res) { 
  	  instrument11.clear(function(err, res) { 
        if ( !err ) {/*
          instrument11.write("D1X", function(err, res) {
              if ( !err ) 
                console.log(res);
          });
          instrument11.write("D4dvaX", function(err, res) {
              if ( !err ) 
                console.log(res);
          });*/
          instrument11.write("D4H1OX", function(err, res) {
              if ( !err ) 
                console.log('OK' + res);
          });        
          /*
          instrument11.query("D1X", function(err, res) {
              if ( !err ) 
                console.log(res);
          });*/
        }
      });
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