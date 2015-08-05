# node-nisa32

nisa32 is another prototype for using VISA  library in node.js.
This is a wrapper for visa32.dll (in your system32 folder).

I tested this code on Windows 8.1 (64bit), Keysight IO Library (17.1), node.js

Talked to HP 6623A...

You must have Keysight or National Instruments VISA drivers installed. You need to have compatible interface and and instrument that talks over GPIB. For simple testing you can skip the hardware part as VISA can talk over COM ports, too. 
```
> npm install node-nisa32

var visa = require('node-nisa32');

visa.query("GPIB0::12::INSTR", "*IDN?", function(err, result) {
 // do something
}
```

Original Author : 7M4MON

