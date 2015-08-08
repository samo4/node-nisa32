'use strict';

var ref = require('ref');
var ffi = require('ffi');
var ArrayType = require('ref-array');
var assert = require("assert");

var EventEmitter = require('events').EventEmitter;
var util = require('util');

// typedef
var ViError = ref.types.int;
var ViSession = ref.types.int;

var byte = ref.types.byte;
var ByteArray = ArrayType(byte);

var visa32 = ffi.Library('visa32', {
	'viOpenDefaultRM': [ViError, ['string'] ] ,							//viOpenDefaultRM(sesn)
	'viOpen' : [ViError, ['int', 'string', 'int', 'int', 'string'] ],	//viOpen(sesn, rsrcName, accessMode, timeout, vi) 
  'viPrintf' : ['int',['int', 'string']],
  'viScanf' : ['int',['int', 'string', ByteArray]],
  'viQueryf' : ['int',['int', 'string', 'string', ByteArray]],
  'viClear' : ['int',['int']],
  'viClose' : ['int', [ViSession] ]
});

var nisa32c = require("../build/nisa32/v1.0.0/Release/node-v14-win32-x64/nisa32");

function VisaPortFactory() {
  var factory = this;
  var resourceManager = '0';
  var viError = 0;
  
  // The default options, can be overwritten in the 'SerialPort' constructor
  var _options = {
    buffersize: 256
  };
  
  function VisaPort(visaAddress, options, openImmediately, callback) {
    assert.equal(typeof (visaAddress), 'string', "argument 'visaAddress' must be a string");
    //assert.equal(typeof (callback), 'function'); 
    var self = this;
    var err;
    options = (typeof options !== 'function') && options || {};
    var opts = {};
    
    openImmediately = (openImmediately === undefined || openImmediately === null) ? true : openImmediately;
  
    if (!visaAddress) {
      err = new Error('Invalid port specified: ' + visaAddress);
      callback(err);
      return;
    }
    
    opts.bufferSize = options.bufferSize || options.buffersize || _options.buffersize;
    this.options = opts;
    
    // intialize Buffer
  	self.replyBuff = new ByteArray(self.options.bufferSize);
  	var counter;
  	for  (counter = 0 ; counter < self.options.bufferSize ; counter++){
  		self.replyBuff [counter] = 0 ;
  	}
    
    self.visaAddress = visaAddress;
    if (openImmediately) {
      /*process.nextTick(function () {
        self.open(callback);
      });*/
    }
  }
  
  VisaPort.prototype.open = function (callback) {
    assert.equal(typeof (callback), 'function');  
    
    viError = visa32.viOpenDefaultRM(resourceManager);
    if (viError) {
    	callback(viError);
    }
    console.log("Opening ADDR : " + this.visaAddress);
    // viOpen: ViSession sesn, ViRsrc name, ViAccessMode mode, ViUInt32 timeout, ViPSession vi
    // c call: viOpen(defaultRM, data->path, VI_NULL, VI_NULL, &instr);
    viError = visa32.viOpen('256', this.visaAddress, '0', '2000', '256');
      if (viError) {
    	return callback(viError);
    }
  }
  
  VisaPort.prototype.query = function (queryString, callback) {
    assert.equal(typeof (queryString), 'string', "argument 'queryString' must be a string");
	  assert.equal(typeof (callback), 'function');

    // we don't initialize the buffer. if the response is empty, we'll get 0 written to the first place anyway.
    var replyBuff = new ByteArray(this.options.bufferSize); 
    viError = visa32.viQueryf('1', queryString + "\r\n", "%s", replyBuff);
    if (viError) {
    	return callback(viError);
    }  
    var replyString = '';
    var counter = 0;
    while(replyBuff[counter] != 0){
    	replyString +=  String.fromCharCode( replyBuff [counter] );
    	counter ++;
    }
    console.log("RECV : " + replyString);
    callback(null, replyString);
  }
  
  VisaPort.prototype.dcl = function(callback) {
    assert.equal(typeof (callback), 'function');
    viError = visa32.viClear('1');
    if (viError) {
    	return callback(viError);
    }  
    else {
      callback(null, true);  
    }
    
  }
  factory.list = nisa32c.list;
  factory.VisaPort = VisaPort;
}

util.inherits(VisaPortFactory, EventEmitter);

module.exports = new VisaPortFactory();