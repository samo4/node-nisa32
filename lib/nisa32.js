'use strict';

var ref = require('ref'); // http://tootallnate.github.io/ref/
var ffi = require('ffi');
var ArrayType = require('ref-array');
var assert = require("assert");

var EventEmitter = require('events').EventEmitter;
var util = require('util');

// typedef
var ViUInt32 = ref.types.int32;
var ViUInt16 = ref.types.uint16;
var ViSession = ViUInt32; 
var ViStatus = ViUInt32;

var byte = ref.types.byte;
var ByteArray = ArrayType(byte);

var visa32 = ffi.Library('visa32', {
	'viOpenDefaultRM': [ViStatus, [ref.types.uint64] ] ,							//viOpenDefaultRM(&sesn where session is memory address )
	'viOpen' : [ViStatus, ['int', 'string', ViUInt32, ViUInt32, ref.types.uint64] ],
  'viWriteAsync': [ViStatus, ['int', ByteArray, 'int', ref.types.uint64]],
  'viGpibControlREN': [ViStatus, [ViSession, ViUInt16]],
  'viPrintf' : [ViStatus,['int', 'string']],
  'viScanf' : [ViStatus,['int', 'string', ByteArray]],
  'viQueryf' : [ViStatus,['int', 'string', 'string', ByteArray]],
  'viClear' : [ViStatus,['int']],
  'viClose' : [ViStatus, [ViSession] ]
});

var nisa32c = require("../build/nisa32/v1.0.0/Release/node-v14-win32-x64/nisa32");

function VisaPortFactory() {
  var factory = this;
  var resourceManager = ref.alloc(ViSession);
  
  // The default options
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
    
    var viError = visa32.viOpenDefaultRM(resourceManager.address());
    if (viError) {
    	callback(viError);
    } else {
      console.log("RM value after:" + resourceManager.deref());
      console.log("Opening VISA resource/instrument: " + this.visaAddress);
      console.log('session value before: ' + this.viSession.deref());
      // viOpen: ViSession sesn, ViRsrc name, ViAccessMode mode, ViUInt32 timeout, ViPSession vi
      viError = visa32.viOpen(resourceManager.deref(), this.visaAddress, '0', this.timeout, this.viSession.address());
      if (viError) {
      	return callback(viError);
      }  else {
        console.log('session value after: ' + this.viSession.deref());
        return callback(null, true);
      }
    }
  }
  
  VisaPort.prototype.write = function (writeString, callback) {
    assert.equal(typeof (callback), 'function');  
    var buf = ref.allocCString(writeString, 'ascii');
    var jobId = ref.alloc(ref.types.int);
    var viError = visa32.viWriteAsync(this.viSession.deref(), buf, buf.length, jobId.address());
    if (viError) {
    	callback(viError);
    } else {
      callback(null, jobId.deref());
    }
  }
  
  VisaPort.prototype.query = function (queryString, callback) {
    assert.equal(typeof (queryString), 'string', "argument 'queryString' must be a string");
	  assert.equal(typeof (callback), 'function');
    console.log("query session id:" + this.viSession.deref());
    // we don't initialize the buffer. if the response is empty, we'll get 0 written to the first place anyway.
    var replyBuff = new ByteArray(this.options.bufferSize); 
    var viError = visa32.viQueryf(this.viSession.deref(), queryString + "\r\n", "%s", replyBuff);
    if (viError) {
    	return callback(viError);
    }  else {
      var replyString = '';
      var counter = 0;
      while(replyBuff[counter] != 0){
      	replyString +=  String.fromCharCode( replyBuff [counter] );
      	counter ++;
      }
      console.log("RECV : " + replyString);
      callback(null, replyString);  
    }
  }
  
  VisaPort.prototype.clear = function(callback) {
    assert.equal(typeof (callback), 'function');
    var viError = visa32.viClear(this.viSession.deref());
    if (viError) {
    	return callback(viError);
    }  
    else {
      callback(null, true);  
    }
  }
  
  /* theoretically.. 
  Send interface clear for 100 microseconds
  
  causes  assert the IFC signal for at least 100 microseconds. 
  This action initializes the GPIB and makes the interface board CIC and Active Controller with ATN asserted, 
  and is generally used when abus fault condition is suspected.
  
  The IFC signal resets only the GPIB interface functions of bus devices and not the internal device functions. 
  Device functions are reset with the Device Clear (DCL) andSelected Device Clear (SDC) commands. 
  */
  VisaPort.prototype.ibsic = function(callback) {
    assert.equal(typeof (callback), 'function');
  }
  
  /* theoretically.. 
  Set or clear the Remote Enable line (REL)
  
  if true, then REN signal is asserted, 
  else, it is deasserted -> Return (all?) devices to local mode
  
  The IBSRE function turns the REN signal on and off. RENis used by devices to select between local and remote
  modes of operation. REN enables the remote mode. A device does not actually enter remote mode until it receives 
  its listen address.
  */
  VisaPort.prototype.ibsre = function(_assert, callback) {
    assert.equal(typeof (callback), 'function');
    assert.equal(typeof (_assert), 'boolean');
    
    var viError = visa32.viGpibControlREN(this.viSession.deref(), _assert);
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