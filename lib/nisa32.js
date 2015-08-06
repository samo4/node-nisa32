'use strict';

var ref = require('ref');
var ffi = require('ffi');
var ArrayType = require('ref-array');
var assert = require("assert");


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
	'viClose' : ['int', [ViSession] ]
});

/* idea:
var self = module.exports 
(for private parts to have direct access to functions)


idea:
 module.exports = function(visaAddress) {
  return new nisa32(visaAddress);
}

function nisa32() {
  var self = this; // Reference to "this" that won't get clobbered by some other "this"
  // Private state variables
  var tempPath;
  // Public method: initialize the object
  self.init = function(options, callback) { ... }
  function private() { ... }
}
*/

/* plan:

var nisa32 = require('./nisa32.js');
var gpib12 = nisa32('GPIB0::12::INSTR');
gpib12.query(cmd, callback);

module.exports = createInstrument;

function createInstrument (visaAddress) {
  assert.equal(typeof (visaAddress), 'string', "argument 'visaAddress' must be a string");
}


----- OR

function Nisa32(visaAddress) {
  this.bufferSize = 
  this.visaAddress = visaAddress;
}

Nisa32.prototype.query = function(queryString, callback) {
  // do stuff on visaAddress with queryString
};

module.exports = Nisa32;
...
var Nisa32 = require("nisa32");
var gpib12 = new Nisa32('GPIB0::12::INSTR'); 


*/

module.exports = {
	query: function (visaAddress, queryString, callback){
		return nisaQuery(visaAddress, queryString, callback);
	}
}

function nisaQuery(visaAddress, queryString, callback){
	assert.equal(typeof (visaAddress), 'string', "argument 'visaAddress' must be a string");
	assert.equal(typeof (queryString), 'string', "argument 'queryString' must be a string");
	assert.equal(typeof (callback), 'function');
	// assert.equal(typeof (timeout), 'number', "argument 'timeout' must be a number");
    // assert.ok(!isNaN(timeout) && timeout > 0, "argument 'timeout' must be a positive integer");
	
	var resourceManager = '0';
	var viError = 0;
	var replyString = '';

	// intialize Buffer
	var replyBuff = new ByteArray(256);
	var counter;
	for  (counter = 0 ; counter < 256 ; counter++){
		replyBuff [counter] = 0 ;
	}

	viError = visa32.viOpenDefaultRM(resourceManager);
	if (viError) {
		callback(viError);
	}

	console.log("ADDR : " + visaAddress + " SEND : " + queryString);
	// viOpen: ViSession sesn, ViRsrc name, ViAccessMode mode, ViUInt32 timeout, ViPSession vi
	// c call: viOpen(defaultRM, data->path, VI_NULL, VI_NULL, &instr);
	viError = visa32.viOpen('256', visaAddress, '0', '2000', '256');
    if (viError) {
		return callback(viError);
	}
	viError = visa32.viPrintf('1', queryString + "\n");
	if (viError) {
		return callback(viError);
	}

	viError = visa32.viScanf('1', "%s", replyBuff);
	if (viError) {
		return callback(viError);
	}
	visa32.viClose(resourceManager);

	// make reply string
	counter = 0;
	while(replyBuff[counter] != 0){
		replyString +=  String.fromCharCode( replyBuff [counter] );
		counter ++;
	}
	console.log("RECV : " + replyString);
	callback(null, replyString);
}