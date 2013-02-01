Gunshot PRO
===========

32bit port of Shotgun BBS Professional version 2 alpha 10.<br />  
Shotgun BBS Professional is Copyright Brent Shellenberg<br />
Gunshot PRO is being ported by Rick Parrish<br />
<br />
The original FILE_ID.DIZ described it as:<br />
<br />
Shotgun BBS Professional version 2 alpha 10.<br />
Complete SVGA/ANSI/ASCII/RIP source of Shotgun Professional. <br />
Bring the touch of high quality Super-VGA to your online service or BBS. <br />
Everything is included that you would normally pay extra for:<br /> 
  * SQUISH/JAM/FIDO Support<br /> 
  * Front End Mailer Software<br /> 
  * TIC File Echo Processor<br /> 
  * Echomail processor<br /> 
  * Multiple Interface (SVGA/ANSI/TTY/RIP) <br />
  * Shotgun GUI Editor included!<br /> 
  * XModem/YModem/ZModem and much much more! <br />
Requires 540k of RAM, 1024k of EMS/XMS, SVGA monitor & mouse.<br />

<hr />

TODO list:<br />
<br />
<ul>
  <li>Implement any REETODOs that appear in compiled executables</li>
  <li>Anything passing 0 for the Attr parameter to FindFirst should pass AnyFile instead (VP returns no files when 0 is passed for Attr)</li>
  <li>Investigate FILEMODE usage to see if FILEMODEREADWRITE, TEXTMODEREAD or TEXTMODEREADWRITE should be used</li>
  <li>Find/correct any usage of FOR loop variables after the loop (since they are 1 greater in VP than in BP</li>
  <li>Rename executables mentioned in code (ie SGECHO to GSECHO in a string in a code file)</li>
</ul>

Completed list:<br />
<br />
<ul>
  <li>IFDEF out anything that doesn't compile and make a WIN32 placeholder that does a "WriteLn('REETODO UNIT FUNCTION'); Halt;" (then you can grep the executables for REETODO to see which REETODOs actually need to be implemented)</li>
  <li>IFDEF out any ASM code blocks and handle the same as above</li>
  <li>WORD in RECORD to SMALLWORD</li>
  <li>INTEGER in RECORD to SMALLINT</li>
  <li>TYPEs of OF WORD to OF SMALLWORD (just in case they're used in a RECORD)</li>
  <li>TYPEs of OF INTEGER to OF SMALLINT (just in case they're used in a RECORD)</li>
</ul>