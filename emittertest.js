/*var binary = require('node-pre-gyp');
var path = require('path');
var PACKAGE_JSON = path.join(__dirname, './package.json');

var binding_path = binary.find(path.resolve(PACKAGE_JSON));
var nisa32c = require(binding_path).Nisa32c; //"./build/nisa32/v1.0.0/Release/node-v14-win32-x64/nisa32");


var EventEmitter = require('events').EventEmitter;

// extend prototype
function inherits(target, source) {
    for (var k in source.prototype) {
        target.prototype[k] = source.prototype[k];
    }
}

inherits(nisa32c, EventEmitter);
*/

var nisa = require('./lib/nisa32.js');
 
var obj = new nisa.VisaPort("ASRL1::INSTR");


Object.getOwnPropertyNames(obj.prototype );

obj.on('event', function() {
    console.log("mijav");
});

// obj.call_emit();
	