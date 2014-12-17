000
001 illegal character
002 comment not closed; started at line %d
003 illegal number
004 string literal not closed or too long
005 unexpected end of file
006 identifier too long
007 identifier expected
008 expected symbol %s
009 illegal keyword usage
010 source text read error
011 can not determine source text language
012 character constant too large (377C or 0FFX is maximum)

020 undeclared identifier "%s"
021 type identifier "%s" shall not be used in declaring itself
022 identifier "%s" was already defined at %s[%d.%d]
023 procedure with forward declaration cannot be a code procedure
024 recursive import not allowed
025 unsatisfied exported object "%s"
026 identifier "%s" is used in its own declaration, see %s[%d.%d]
027 illegal usage of module identifier "%s"
028 identifier "%s" was already defined in other module
029 incompatible types:\n    "%s"\n    "%s"
030 incompatible types
031 identifier does not denote a type
032 scalar type expected
033 ordinal type expected
034 illegal type conversion
035 NEW: "%s" not known in this scope
036 DISPOSE: "%s" not known in this scope
037 procedure "%s" should be a proper procedure
038 illegal number of parameters "%s"
039 procedure "%s": %s parameter expected for "%s"
040 procedure "%s": type of parameter "%s" mismatch
041 guard or test type is not an extension of variable type

043 illegal result type of procedure
044 incompatible result types

046 illegal usage of open array type
047 fewer actual than formal parameters
048 more actual than formal parameters
049 sequence parameter should be of SYSTEM.BYTE or SYSTEM.LOC type
050 object is not array
051 object is not record
052 object is not pointer
053 object is not set
054 object is not variable
055 object is not procedure: %s

057 a call of super method is valid in method redifinition only
058 type-bound procedure is not defined for the base type
059 object is neither a pointer nor a VAR-parameter record
060 pointer not bound to record or array type
061 dimension too large or negative
062 pointer not bound to record

064 base type of open array aggregate should be a simple type
065 the record type is from another module

067 receiver type should be exported %s
068 this type-bound procedure cannot be called from a record
069 wrong mode of receiver type

071 non-Oberon type cannot be used in specific Oberon-2 construct
072 illegal order of redefinition of type-bound procedures

074 redefined type-bound procedure should be exported
075 function procedure without RETURN statement
076 value is required

078 SIZE (TSIZE) cannot be applied to an open array
079 HIGH or LEN cannot be applied to a foreign open array

081 expected start of factor
082 expected start of declaration
083 expected start of type
085 expected expression
086 expected start of statement
087 expression should be constant
088 identifier does not match block name
089 procedure not implemented "%s"
090 proper procedure is expected
091 call of proper procedure in expression
092 code procedure is not allowed in definition module
093 not allowed in definition module
094 allowed only in definition module
095 allowed only in global scope
096 unsatisfied opaque type "%s"
097 unsatisfied forward type "%s"
098 allowed only for value parameter
099 RETURN allowed only in procedure body
100 illegal order of declarations

102 language extension is not allowed %s
103 INTERNAL ERROR(ME): value expected
104 INTERNAL ERROR(ME): designator expected
105 INTERNAL ERROR(ME): statement expected
106 INTERNAL ERROR(ME): node type = NIL
107 shall not have a value less than 0
108 CASE in record constructor is not allowed

109 forward type cannot be opaque
110 illegal length, %d was expected
111 repetition counter must be an expression of a whole number type
112 expression for field "%s" was expected
113 no variant is associated with the value of the expression
114 object "%s" is a field

116 field "%s" is not exported
117 type of "%s" is not defined yet
118 base type is allowed only for Oberon record
119 variant fields are not allowed in Oberon record
120 field of Oberon type is not allowed in non-Oberon record
121 illegal use of type designator "%s"
122 expression out of bounds
123 designator is read-only
124 low bound greater than high bound
125 EXIT not within LOOP statement
126 case label defined more then once

128 FOR-loop control variable must be declared in the local scope
129 more expressions than fields in a record type

131 zero step in FOR statement
132 shall be an open array designator
133 implementation limit exceeded for set base type (length > %d)
134 must be value of unsigned type
135 must be value of pointer type
136 must be type designator
137 numeric constant does not have a defined storage size
138 string literals are not allowed here

139 must be (qualified) identifier which denotes variable
140 interrupt procedures are not implemented yet
141 opaque type can not be defined as Oberon pointer
142 INTERNAL ERROR(ME): can not generate code
143 not allowed in Oberon
144 pointer and record types are mixed in type test
145 control variable must not be a formal parameter
146 control variable cannot be exported
147 threatened variable is not allowed here
148 finalization is allowed only in module block
149 RETRY is allowed only in exceptional part of block
150 invalid system flag value
151 must be of integer type
152 incompatible calling conventions:\n    "%s"\n    "%s"
153 incompatible calling conventions
154 procedure "%s" does not match previous definition:\n  was: %s\n  now: %s
155 procedure "%s" does not match previous definition
156 procedure designator is expected
157 object must be exported
158 control variable "%s" is threatened here
159 type of aggregate is not set or array or record
160 invalid parameter specification: expected NIL
161 VAR [NIL] parameter expected
162 %s value should be in %{} (not "%s")
163 control variable can not be volatile
164 Language is not valid for external procedure

171 illegal structure of conditional compilation options
172 conditional compilation option starts with incorrect symbol
173 pragma not closed; started at line %d
174 unexpected end of file while skipping; see at %d
175 invalid pragma syntax
176 "%s" illegal value for equation %s

190 incorrect header in symbol file "%s"
191 incorrect version of symbol file "%s" (%d instead of %d)
192 key inconsistency of imported module "%s"
193 generation of new symbol file not allowed
194 module name does not match symbol file name "%s"
195 cannot read symbol file "%s" generated by %s
196 INTERNAL ERROR: incorrect sym ident %d while reading symbol file "%s"
197 INTERNAL ASSERT(%d) while reading symbol file "%s"

200 not yet implemented
201 real overflow or underflow in constant expression
202 integer overflow in constant expression
203 division by zero

206 array length is too large or less then zero

208 CASE statement always fails

219 too many dimensions (implementation limit %d)
220 heirarchy of record extensions too high (implementation limit %d)
221 procedure declaration nesting limit (%d) has been exceeded

281 type-bound procedure is not valid as procedure value
282 local procedure is not valid as procedure value "%s"
283 code (or external) procedure is not valid as procedure value

285 label declared but not used
286 object is not a label
287 label is not declared in the current scope
288 label expected
289 label already defined

290 Foreign module may not have a body

% ------- WARNINGS

300 variable "%s" declared but never used
301 parameter "%s" is never used
302 value was assigned but never used
303 procedure "%s" declared but never used
304 possibly used before definition "%s"
305 constant declared but never used
306 import of "%s" is never used
307 needless parameter "%s" is used

310 infinite loop
311 unreachable code
312 loop is executed exactly once

314 variable "%s" has compile time defined value here
315 NIL dereference
316 this SYSTEM procedure is not described in Modula-2 ISO standard
317 VAR parameter is used here, check that it is not threatened inside WITH
318 redundant FOR statement
319 CASE statement without ELSE
320 undeclared option "%s"
321 option "%s" is already defined
322 undeclared equation "%s"
323 equation "%s" is already defined
324 field %s alignment (%d) is greater then record alignment (%d)
325 possible ambiguity; two-char array passed
326 possible side effect in removed ASSERT argument

330 function %s inlined
331 external function %s inlined

341 Import of module with body to module without body

% ------- C backend warnings
350 non portable type cast: size is undefined
351 option NOHEADER is allowed only in C-modules
352 option NOCODE is allowed only in C-modules
353 dependence cycle in C code

390 obsolete pragma setting

% ------- make errors

400 #definition or Oberon module is not found for module "%s"
401 #implementation is not found for module "%s"
402 #compile "%s" ("%s" not found)
403 #compile "%s" ("%s" was modified after "%s")
404 #compile "%s" ("%s" was modified)
405 #compile "%s" (was modified after "%s")
406 #undefined file extension "%s"
407 #module not found "%s"
408 #file "%s": definition module expected
409 #file "%s": main module expected
410 #file "%s": Oberon module expected
411 #file "%s": implementation or main module expected
412 #file "%s": module name "%s" expected, got "%s"
413 #no modules in the project
414 #file "%s" (line %d): %s
415 #unknown operation mode "%s"
416 #inconsistent set of operation modes
417 #file "%s": module name "%s" expected (import from "%s")
418 string expected
419 unknown extension "%s"
420 unknown template argument "%s"
421 only one level of "{}" brackets is allowed
422 inconsistent set of modules kind
423 #more than one source file for module:\n  %s\n  %s
424 file create error: %s
425 file open error: %s
426 #unexpected module kind: "%s"
427 #file "%s": error in module header or import section
428 syntax error
429 line too long
430 #invalid argument (%s): %s
431 #ERRFMT equation: wrong syntax (position %d)
432 file close error: %s
433 #error in template file "%s", line %d.%d: %s
434 #wrong value of equation "%s": "%s" is used
435 #project is up to date
436 #cycle in import (See "%s", "%s")
437 #too many errors
438 is allowed only inside "{}" brackets
439 external command fault %d:\n%s
440 #error in template string, line %d.%d: %s\n%s
441 #file "%s" (line %d.%02d): %s
442 incompatible types
443 undeclared identifier
444 #file "%s": implementation module expected
445 #file "%s": read error
446 "end" missed
447 could not execute external command:\n%s
448 file write error: %s

450 compilation aborted: %s

500 can not output debug info
501 can not output object file

% ------- x86 compiler warnings
900 redundant code eliminated
901 redundant code not eliminated - can raise exception
902 constant condition eliminated
903 function result is not used

910 realValueException will be raised here
911 wholeValueException will be raised here
912 wholeDivException will be raised here
913 indexException will be raised here
914 rangeException will be raised here
915 invalidLocation exception will be raised here
916 realDivException will be raised here

% ------- x86 compiler errors
920 INTERNAL ERROR: cardinal overflow
921 invalid code procedure
922 code procedures not supported

% ------- x86 compiler fatal errors
950 out of memory
951 expression(s) too complex
952 that type conversion is not implemented

960 INTERNAL ERROR: invalid constant value
961 INTERNAL ERROR: invalid type mode (%d)
962 INTERNAL ERROR: invalid object mode (%d)

% ------- C backend errors
1000 can not gen. overlapped field, GENCPP is OFF
1001 parameter "%s" is not declared
1002 can not generate recursive type definition
1003 external names conflict:\n    "%s.%s" and "%s.%s"
1004 external name "%s.%s" conflict with standard name (xm.kwd)
1005 unimplemented system procedure
1006 undefined array length for dimension %d
1007 undefined array size for dimension %d
1008 can not generate expression
1009 can not generate l-value type cast
1010 can not generate type conversion
1011 can not generate aggregate
1012 can not generate statement
1013 cannot generate constant aggregate of this type
1014 size of %s exceeds the limit
1015 too many parameters (implementation limit)
1016 can not generate type designator
1017 can not generate type declaration
1018 can not generate recursive type declaration
1019 can not generate object declaration

% ------- Embedded X86 assembler messages
3001 expression too complex
3002 ")" expexted
3003 expression expected
3004 "PTR" expected
3005 value is out of range
3006 illegal address mode
3007 number required
3008 illegal scale value
3009 illegal argument
3010 immediate value is not allowed here
3011 "," expected
3012 unallowed operand size
3013 unknown operand size
3014 different operand size
3015 register expected
3016 32-bit register expected
3017 CS is not allowed here
3018 16-bit operand expected
3019 only CL register is allowed here
3020 8-bit register is not allowed here
3021 8-bit operand is not allowed here
3022 label expected
3023 immediate value expected
3024 unknown command
3025 extra characters after end of command
3026 label redefinition
3027 undefined label
3028 syntax error
3029 label too far
3030 undefined identifier
3031 identifier expected
3032 illegal qualified identifier
3033 constant required here
3034 illegal constant type
3035 label or procedure name expected
3036 only AL/AX/EAX are allowed here
3037 only DX register is allowed here
3038 registers except "AX" are not allowed here
3039 FP-register expected
3040 "ST(0)" expected
3041 MMX-register expected
3042 ":" expected
3043 instruction allowed only in code section
3044 constant not allowed here
3045 reference to non data label
3046 reference to non code label
3047 record type or variable expected
3048 "." expected
3049 illegal constant size
3050 using of two variable in same expression is illegal (use OFFSET)
3051 using of two reference to global variables in same expression is illegal
3052 illegal variable usage
3053 there is no correspondent left parenthesis
3054 unknown size
3201 16-bit address mode is not implemented
3301 assembler internal error #1
