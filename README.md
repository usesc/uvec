# uvec v2
micro vector implementation in C

-- SEPTEMBER YIELDS MAJOR REFACTOR: VERSION 2 LIVE --

* Better growth strategies
* umalloc, urealloc, ufree, growthfunc, shrinkfunc all modifiable
* int (4 bytes) return codes to char (1 byte)
* increased code readability massively 
* dropped macros
* faster math (probably not noticeable)
* tab width tabs (one byte of 0x09) 
