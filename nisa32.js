var ref = require('ref');
var ffi = require('ffi');
var ArrayType = require('ref-array');


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

exports.query = function (visaAddress, queryString, callback){
	return nisaQuery(visaAddress, queryString, callback);
};

function nisaQuery(visaAddress, queryString, callback){

	var resourceManager = '0';
	var viError = 0;
	var session = 0;
	var replyString = '';

	// intialize Buffer
	var replyBuff = new ByteArray(256);
	var counter;
	for  (counter = 0 ; counter < 256 ; counter++){
		replyBuff [counter] = 0 ;
	}

	viError = visa32.viOpenDefaultRM('0');
	if (viError) {
		callback(viError);
	}

	console.log("ADDR : " + visaAddress + " SEND : " + queryString);
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