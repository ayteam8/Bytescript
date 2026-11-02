# Bytescript Standard library

Content :
- [*Built-ins*](#built-ins)
    - [Globals](#globals)
    - [Table](#table)
    - [String](#string)
    - [IO](#io)
    - [File](#file)
    - [Error](#error)  
- [*stdlib*](#standard-library)
    - [Base64](#base64)
    - [Debug](#debug)
    - [Json](#json)
    - [Math](#math)
    - [OS](#os)
    - [Random](#random)
    - [Time](#time)

## Built-ins

Built-ins modules do not need to be imported by the user and can be accessed at any time  

### Globals

- `print(var value)`  
**Signature** : `func var print(var value) end`  
**Description** : Prints a value in stdout  
**Arguments** : 
    - `value` : The value to print

- `input(String text)`   
**Signature** : `func var input(String text) end`    
**Description** : Reads a line from `stdin`   
**Arguments**   
    - `text` : The text to print   

- `free(var value)`  
**Signature** : `func var free(var value) end`  
**Description** : Frees a heap allocated value  
**Arguments** : 
    - `value` : The value to free
  
- `disassemble(Function func, String output)`  
**Signature** : `func var disassemble(Function func, String output) end`  
**Description** : Writes the bytecode of a given function in the given file  
**Arguments** : 
    - `func` : The function to disassemble
    - `output` : The path to the file you wish to write the bytecode in, **optional**, *defaults to IO.stdout*
  
- `exec(String src)`  
**Signature** : `func var exec(String src) end`  
**Description** : Execute the given source code  
**Arguments** : 
    - `src` : The code to execute
  

### Table

- `Table:new()`  
**Signature** : `func static Table __new() end`  
**Description** : Creates an empty table and returns it  
**Arguments** : None  
  
- `Table:free()`  
**Signature** : `func Table free() end`  
**Description** : Frees a table  
**Arguments** : None  
  
- `Table:pop(Number index)`  
**Signature** : `func pop(Number index) end`  
**Description** : Creates an empty table  
**Arguments** :
    - `index` : The index of the value you wish to remove from the table, **optional**, *defaults to the last index*
  
- `Table:shrink()`  
**Signature** : `func Table shrink() end`  
**Description** : Shrinks a table's capacity  
**Arguments** : None  
  
- `Table:expand()`  
**Signature** : `func Table expand() end`  
**Description** : Expands a table's capacity  
**Arguments** : None  
  
- `Table:len()`  
**Signature** : `func Number len() end`  
**Description** : Returns the lenght of the table  
**Arguments** : None  
  
- `Table:reserve(Number amount)`    
**Signature** : `func Number reserve(Number amount) end`  
**Description** : Expands a table's capacity to match your needs  
**Arguments** :
    - `amount` : The amount of value you wish to reserve
  
- `Table:print()`  
**Signature** : `func Table print() end`  
**Description** : Nicely prints a table  
**Arguments** : None  
  
- `Table:slice(Number start, Number end)`  
**Signature** : `func Table slice(Number start, Number end) end`  
**Description** : Returns a sub-table of the original table  
**Arguments** : 
    - `start` : The index at wich you wish to begin the sub-table
    - `end` : the index at wich you wish to end the sub-table, must be bigger than `start` 
  
- `Table:fill(value)`  
**Signature** : `func Table fill(value) end`  
**Description** : Fills a table with the given value  
**Arguments** : 
    - `value` : the value to fill the table with
  
- `Table:copy()`  
**Signature** : `func Table copy() end`  
**Description** : Creates a copy of the table and returns it  
**Arguments** : None  
  
- `Table:insert(value, Number idx)`  
**Signature** : `func insert(value, Number idx) end`  
**Description** : Add a value to the table's array part at a given index  
**Arguments** : 
    - `value` : The value to add to the table
    - `idx` : The index at wich you wish to add the value, **optional** defaults to the last index
  
- `Table:appendHash(key, value)`  
**Signature** : `func appendHash(key, value) end`  
**Description** : Add a value to the table's hash-table/dictionnary part  
**Arguments** : 
    - `key` : The key to store the value under
    - `value` : The value to store
  
- `Table:get(key)`  
**Signature** : `func get(key) end`  
**Description** : Gets the value corresponding to the given key from the table's hash-table/dictionnary part, returns NIL if not found  
**Arguments** : 
    - `key` : the key that the value was stored under
  
- `Table:clean()`  
**Signature** : `func Table clean() end`  
**Description** : Cleans the table by removing its elements  
**Arguments** : None  
  
- `Table:reset()`  
**Signature** : `func Table reset() end`  
**Description** : Resets a table by allocating a brand new array and hash-table  
**Arguments** : None  
  
- `Table:hashedKeys()`  
**Signature** : `func Table hashedKeys() end`  
**Description** : Returns all the keys stored in the table's hash-table/dictionnary  
**Arguments** : None  
  
- `Table:hashedValues()`  
**Signature** : `func Table hashedValues() end`  
**Description** : Returns all the values stored in the table's hash-table/dictionnary  
**Arguments** : None  
  
- `Table:buckets()`  
**Signature** : `func Table buckets() end`  
**Description** : Returns all the buckets (key, value) stored in the table's hash-table/dictionnary  
**Arguments** : None  
  
- `Table:inArray()`  
**Signature** : `func Number inArray(value) end`  
**Description** : Checks if a value is in the table's array part  
**Arguments** : None  
  
- `Table:hashValue(value)`  
**Signature** : `func static hashValue(value) end`  
**Description** : Returns the hashed version of the value  
**Arguments** : 
    - `value` : The value to hash
  
  
### String

- `String:compare(String other)`  
**Signature** : `func Number compare(String other) end`  
**Description** : Compares the string with another one  
**Arguments** : 
    - `other` : The string to compare with

- `String:format(var value)`  
**Signature** : `func String format(var value) end`  
**Description** : Formats a string using format specifiers  
**Arguments** : 
    - `value` : The value to format

- `String:len()`  
**Signature** : `func Number len() end`  
**Description** : Returns the lenght of the string  
**Arguments** : None  

- `String:toString(var value)`  
**Signature** : `func static String toString(var value) end`  
**Description** : Turn a value into a string  
**Arguments** : 
    - `value` : The value to cast into a string


### IO

- `IO.stdout`  
**Description** : A File wrapper to the standard output  

- `IO:open(String filename, String openType)`  
**Signature** : `func static File open(String filename, String openType) end`  
**Description** : opens a file with the specified openType, returns a File object  
**Arguments** : 
    - `filename` : The path to the file
    - `openType` : The open type to open the file with


### File

- `File:read()`  
**Signature** : `func String read() end`  
**Description** : Reads a file and returns a string with its content  
**Arguments** : None

- `File:write(String content)`  
**Signature** : `func String write(String content) end`  
**Description** : Writes content into a file  
**Arguments** : 
    - `content` : The content to write into the file

- `File:close()`  
**Signature** : `func close() end`    
**Description** : Closes the file  
**Arguments** : None

### Error

- `Error:new(String name, String details)`  
**Signature** : `func static Error __new(String name, String details) end`      
**Description** : Calls an Error with the specified name and details  
**Arguments** :  
    - `name` : the name to call the error with
    - `details` : the details of the error

- `TypeError:new(String details)`  
**Signature** : `func static TypeError __new(String details) end`      
**Description** : Calls a TypeError with the specified details  
**Arguments** :  
    - `details` : the details of the error

- `IndexError:new(String details)`  
**Signature** : `func static IndexError __new(String details) end`      
**Description** : Calls an IndexError with the specified details  
**Arguments** :  
    - `details` : the details of the error

## Standard library

Modules that needs to be imported, they come with bytescript in the `stdlib` folder  


### Base64

- `Base64:encode(String text)`  
**Signature** : `func String encode(String text) end`  
**Description** : Encode a string into base64  
**Arguments** : 
    - `text` : The text to encode  

- `Base64:decode(String text)`  
**Signature** : `func String decode(String text) end`  
**Description** : Decode a base64 encoded string  
**Arguments** : 
    - `text` : The text to decode  


### Debug

- `AssertionError:new(String details)`  
**Signature** : `func static AssertionError __new(String details) end`      
**Description** : Calls an AssertionError with the specified details  
**Arguments** :  
    - `details` : the details of the error


- `Debug:assert(Number condition, String message)`  
**Signature** : `func var assert(Number condition, String message) end`  
**Description** : Throw an AssertionError error if the given condition is falsy  
**Arguments** : 
    - `message` : The error message to show, **optional**


### Json

- `Json:parse(String json)`  
**Signature** : `func Table parse(String json) end`  
**Description** : Parse a json string into a bytescript table  
**Arguments** : 
    - `json` : The string to parse

- `Json:stringify(var value)`  
**Signature** : `func String stringify(var value) end`  
**Description** : Stringifies the given value into a json string  
**Arguments** : 
    - `value` : The value to stringify

- `Json:valid(String json)`  
**Signature** : `func Number valid(String json) end`  
**Description** : Returns true if the json string is valid, otherwise returns false  
**Arguments** : 
    - `json` : The string to validify

- `Json:load(String path)`  
**Signature** : `func Table load(String path) end`  
**Description** : Parse a json file into a bytescript table  
**Arguments** : 
    - `path` : The path to the json file

- `Json:write(String path, Table json)`  
**Signature** : `func Number write(String path, Table json) end`  
**Description** : Write a bytescript table into a json file  
**Arguments** : 
    - `path` : The path to the json file
    - `json` : The table to write into the file  


### Math

- `Math.pi`  
**Description** : it's pi  

- `Math.e`  
**Description** : it's e  

- `Math.inf`  
**Description** : it's infinity  

- `Math:max(Number a, Number b)`  
**Signature** : `func Number max(Number a, Number b) end`  
**Description** : Returns the bigger number between a and b  
**Arguments** : 
    - `a` : The first number
    - `b` : The second number

- `Math:max(Number a, Number b)`  
**Signature** : `func Number max(Number a, Number b) end`  
**Description** : Returns the bigger number between a and b  
**Arguments** : 
    - `a` : The first number
    - `b` : The second number

- `Math:min(Number a, Number b)`  
**Signature** : `func Number min(Number a, Number b) end`  
**Description** : Returns the smaller number between a and b  
**Arguments** : 
    - `a` : The first number
    - `b` : The second number

- `Math:clamp(Number a, Number b, Number c)`  
**Signature** : `func Number clamp(Number a, Number b, Number c) end`  
**Description** : Clamps c between a and b  
**Arguments** : 
    - `a` : The first number
    - `b` : The second number
    - `c` : The number to clamp

- `Math:abs(Number a)`  
**Signature** : `func Number abs(Number a) end`  
**Description** : Returns the absolute value of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:floor(Number a)`  
**Signature** : `func Number floor(Number a) end`  
**Description** : Returns the floor value of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:ceil(Number a)`  
**Signature** : `func Number ceil(Number a) end`  
**Description** : Returns the ceiling value of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:round(Number a)`  
**Signature** : `func Number round(Number a) end`  
**Description** : Returns the rounded value of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:trunc(Number a)`  
**Signature** : `func Number trunc(Number a) end`  
**Description** : Returns the truncated value of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:sqrt(Number a)`  
**Signature** : `func Number sqrt(Number a) end`  
**Description** : Returns the square root of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:sin(Number a)`  
**Signature** : `func Number sin(Number a) end`  
**Description** : Returns the sine of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:cos(Number a)`  
**Signature** : `func Number cos(Number a) end`  
**Description** : Returns the cosine of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:tan(Number a)`  
**Signature** : `func Number tan(Number a) end`  
**Description** : Returns the tangent of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:asin(Number a)`  
**Signature** : `func Number asin(Number a) end`  
**Description** : Returns the arcsine of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:acos(Number a)`  
**Signature** : `func Number acos(Number a) end`  
**Description** : Returns the arccosine of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:atan(Number a)`  
**Signature** : `func Number atan(Number a) end`  
**Description** : Returns the arctangent of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:atan2(Number a, Number b)`  
**Signature** : `func Number atan2(Number a, Number b) end`  
**Description** : Returns the arctangent of the given vector without information loss  
**Arguments** : 
    - `a` : The vector's x coordinate
    - `b` : The vector's y coordinate

- `Math:exp(Number a)`  
**Signature** : `func Number exp(Number a) end`  
**Description** : Returns the exponential value of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:log(Number a)`  
**Signature** : `func Number log(Number a) end`  
**Description** : Returns the natural logarithmic value of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:log10(Number a)`  
**Signature** : `func Number log10(Number a) end`  
**Description** : Returns the base 10 logarithmic value of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:sign(Number a)`  
**Signature** : `func Number sign(Number a) end`  
**Description** : Returns the sign of the given number  
**Arguments** : 
    - `a` : The target number

- `Math:is_inf(Number a)`  
**Signature** : `func Number abs(Number a) end`  
**Description** : Returns true if the given value is infinity otherwise false  
**Arguments** : 
    - `a` : The target number

- `Math:deg(Number a)`  
**Signature** : `func Number deg(Number a) end`  
**Description** : Converts the given radiants into degrees and returns it  
**Arguments** : 
    - `a` : The target number

- `Math:rad(Number a)`  
**Signature** : `func Number rad(Number a) end`  
**Description** : Converts the given degrees into radiants and returns it  
**Arguments** : 
    - `a` : The target number


### OS

- `OS:env(String name)`  
**Signature** : `func String env(String name) end`  
**Description** : Returns content of the specified environnement  
**Arguments** : 
    - `name` : The name of the target environnement

- `OS:cwd()`  
**Signature** : `func String cwd() end`  
**Description** : Returns the current working directory  
**Arguments** : None  

- `OS:chdir(String path)`  
**Signature** : `func String env(String path) end`  
**Description** : Changes the current working directory into the given path  
**Arguments** : 
    - `path` : The new working directory

- `OS:absolute(String path)`  
**Signature** : `func String absolute(String path) end`  
**Description** : Returns the absolute path of the given location  
**Arguments** : 
    - `path` : The path to the directory/file you want the absolute path to

- `OS:filename(String path)`  
**Signature** : `func String filename(String path) end`  
**Description** : Returns the filename of a given file's path  
**Arguments** : 
    - `path` : The path to the target file

- `OS:stem(String path)`  
**Signature** : `func String stem(String path) end`  
**Description** : Returns the stem of a given file's path (filename without extention)  
**Arguments** : 
    - `path` : The path to the target file

- `OS:ext(String path)`  
**Signature** : `func String ext(String path) end`  
**Description** : Returns the extention of a given file's path  
**Arguments** : 
    - `path` : The path to the target file

- `OS:parent(String path)`  
**Signature** : `func String parent(String path) end`  
**Description** : Returns the parent of the given file  
**Arguments** : 
    - `path` : The path to the target file

- `OS:join(String a, String b)`  
**Signature** : `func String join(String a, String b) end`  
**Description** : Joins 2 paths into one  
**Arguments** : 
    - `a` : The first part of the target path
    - `b` : The second part of the target path

- `OS:run(String command)`  
**Signature** : `func Number run(String command) end`  
**Description** : Runs a command and returns the exit code  
**Arguments** : 
    - `run` : The command to execute

- `OS:exit(Number code)`  
**Signature** : `func var exit(Number code) end`  
**Description** : exit the program with the given exit code  
**Arguments** : 
    - `code` : The exit code to exit with


### Random

- `Random:seed()`  
**Signature** : `func var seed() end`  
**Description** : set a new random seed  
**Arguments** : None  

- `Random:random()`  
**Signature** : `func Number random() end`  
**Description** : Returns a random number between 0 and 1  
**Arguments** : None  

- `Random:range()`  
**Signature** : `func Number range(Number min, Number max) end`  
**Description** : Returns a random number in the given range  
**Arguments** : 
    - `min` : the minimum number that can be generated
    - `max` : the maximum number that can be generated

- `Random:integer()`  
**Signature** : `func Number integer(Number min, Number max) end`  
**Description** : Returns a random integer in the given range  
**Arguments** : 
    - `min` : the minimum number that can be generated
    - `max` : the maximum number that can be generated


### Time

- `Time:time()`  
**Signature** : `func Number time() end`  
**Description** : Returns the current time of the day  
**Arguments** : None  

- `Time:clock()`  
**Signature** : `func Number clock() end`  
**Description** : Returns a high precision value corresponding to the current cpu cycle  
**Arguments** : None  

- `Time:sleep()`  
**Signature** : `func var sleep(Number milliseconds) end`  
**Description** : Pause the program for a specific amount of time  
**Arguments** : 
    - `milliseconds` : the amount of time to pause to program for, in milliseconds

