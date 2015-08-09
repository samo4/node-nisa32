'use strict';

var ref = require('ref');
var ffi = require('ffi');
var ArrayType = require('ref-array');
var assert = require("assert");

var EventEmitter = require('events').EventEmitter;
var util = require('util');

// typedef
var ViError = ref.types.int;
var ViUInt32 = ref.types.int32;
var ViSession = ViUInt32; //// Visession is viobject is ViUInt32

var byte = ref.types.byte;
var ByteArray = ArrayType(byte);

var visa32 = ffi.Library('visa32', {
	'viOpenDefaultRM': [ViError, [ref.types.uint64] ] ,							//viOpenDefaultRM(&sesn where session is memory address )
	'viOpen' : [ViError, ['int', 'string', ViUInt32, ViUInt32, ref.types.uint64] ],	
  'viPrintf' : ['int',['int', 'string']],
  'viScanf' : ['int',['int', 'string', ByteArray]],
  'viQueryf' : ['int',['int', 'string', 'string', ByteArray]],
  'viClear' : ['int',['int']],
  'viClose' : ['int', [ViSession] ]
});

var nisa32c = require("../build/nisa32/v1.0.0/Release/node-v14-win32-x64/nisa32");

function VisaPortFactory() {
  var factory = this;
  var resourceManager = ref.alloc(ViSession);
  var viError = 0;
  
  // The default options, can be overwritten in the 'VisaPort' constructor
  var _options = {
    buffersize: 256
  };
  
  function VisaPort(visaAddress, options, openImmediately, callback) {
    assert.equal(typeof (visaAddress), 'string', "argument 'visaAddress' must be a string"); 
    var self = this;
    var err;
    options = (typeof options !== 'function') && options || {};
    var opts = {};
    
    openImmediately = (openImmediately === undefined || openImmediately === null) ? false : openImmediately;
    if(openImmediately) {
      assert.equal(typeof (callback), 'function', 'If you want to openImmediately, you need to define a callback...');
    }
  
    if (!visaAddress) {
      err = new Error('Invalid port specified: ' + visaAddress);
      callback(err);
      return;
    }
    
    opts.bufferSize = options.bufferSize || options.buffersize || _options.buffersize;
    this.options = opts;
    
    // intialize Buffer
    /*
  	self.replyBuff = new ByteArray(self.options.bufferSize);
  	var counter;
  	for  (counter = 0 ; counter < self.options.bufferSize ; counter++){
  		self.replyBuff [counter] = 0 ;
  	}*/
    self.timeout = 2000;
    self.viSession = ref.alloc(ViSession);
    self.visaAddress = visaAddress;
    if (openImmediately) {
      process.nextTick(function () {
        self.open(callback);
      });
    }
  }
  
  VisaPort.prototype.open = function (callback) {
    assert.equal(typeof (callback), 'function');  
    console.log("RM address:" + resourceManager.address() + "; value before open:" + resourceManager.deref());
    
    viError = visa32.viOpenDefaultRM(resourceManager.address());
    if (viError) {
    	callback(viError);
    } else {
      console.log("RM value after:" + resourceManager.deref());
      console.log("Opening VISA resource/instrument: " + this.visaAddress);
      console.log('session value before: ' + this.viSession.deref());
      // viOpen: ViSession sesn, ViRsrc name, ViAccessMode mode, ViUInt32 timeout, ViPSession vi
      // c call: viOpen(defaultRM, data->path, VI_NULL, VI_NULL, &instr);
      viError = visa32.viOpen(resourceManager.deref(), this.visaAddress, '0', this.timeout, this.viSession.address());
        if (viError) {
      	return callback(viError);
      }  else {
        console.log('session value after: ' + this.viSession.deref());
        return callback(null, true);
      }
    }
  }
  
  VisaPort.prototype.query = function (queryString, callback) {
    assert.equal(typeof (queryString), 'string', "argument 'queryString' must be a string");
	  assert.equal(typeof (callback), 'function');
    console.log("query session id:" + this.viSession.deref());
    // we don't initialize the buffer. if the response is empty, we'll get 0 written to the first place anyway.
    var replyBuff = new ByteArray(this.options.bufferSize); 
    viError = visa32.viQueryf(this.viSession.deref(), queryString + "\r\n", "%s", replyBuff);
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
    viError = visa32.viClear(this.viSession.deref());
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