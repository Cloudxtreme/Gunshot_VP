{$I DEFINES.INC}
UNIT SaveScrn;


INTERFACE


PROCEDURE Save_Screen;
PROCEDURE Restore_Screen;
PROCEDURE Save_Screen2;
PROCEDURE Restore_Screen2;
PROCEDURE Save_Screen3;
PROCEDURE Restore_Screen3;


IMPLEMENTATION


USES Globals;


PROCEDURE Save_Screen;
BEGIN
{$IFDEF MSDOS}
     Move(MEM[SegB800:0],SaveScreen1,4000);
{$ENDIF}
{$IFDEF WIN32}
  WriteLn('REETODO SAVESCRN Save_Screen'); Halt;
{$ENDIF}
END;


PROCEDURE Restore_Screen;
BEGIN
{$IFDEF MSDOS}
     Move(SaveScreen1,MEM[SegB800:0],4000);
{$ENDIF}
{$IFDEF WIN32}
  WriteLn('REETODO SAVESCRN Restore_Screen'); Halt;
{$ENDIF}
END;


PROCEDURE Save_Screen2;
BEGIN
{$IFDEF MSDOS}
     Move(MEM[SegB800:0],SaveScreen2^,4000);
{$ENDIF}
{$IFDEF WIN32}
  WriteLn('REETODO SAVESCRN Save_Screen2'); Halt;
{$ENDIF}
END;


PROCEDURE Restore_Screen2;
BEGIN
{$IFDEF MSDOS}
     Move(SaveScreen2^,MEM[SegB800:0],4000);
{$ENDIF}
{$IFDEF WIN32}
  WriteLn('REETODO SAVESCRN Restore_Screen2'); Halt;
{$ENDIF}
END;


PROCEDURE Save_Screen3;
BEGIN
{$IFDEF MSDOS}
     Move(MEM[SegB800:0],SaveScreen3^,4000);
{$ENDIF}
{$IFDEF WIN32}
  WriteLn('REETODO SAVESCRN Save_Screen3'); Halt;
{$ENDIF}
END;


PROCEDURE Restore_Screen3;
BEGIN
{$IFDEF MSDOS}
     Move(SaveScreen3^,MEM[SegB800:0],4000);
{$ENDIF}
{$IFDEF WIN32}
  WriteLn('REETODO SAVESCRN Restore_Screen3'); Halt;
{$ENDIF}
END;


END.