{$I DEFINES.INC}
UNIT SaveScrn;


INTERFACE


PROCEDURE Save_Screen2;
PROCEDURE Restore_Screen2;
PROCEDURE Save_Screen;
PROCEDURE Restore_Screen;


IMPLEMENTATION


USES Globals,Crt,ANSIMisc,BsKeys{$IFDEF WIN32},RPScreen{$ENDIF};


{$IFDEF WIN32}
var
  SavedScreen1: TScreenBuf;
  SavedScreen2: TScreenBuf;
{$ENDIF}

{LOCAL SCREEN SAVE}


PROCEDURE Save_Screen2;
BEGIN
{$IFDEF MSDOS}
     Move(MEM[SegB800:0],SAVESCRN_Scr2[1],4000);
{$ENDIF}
{$IFDEF WIN32}
     RPSaveScreen(SavedScreen2);
{$ENDIF}
END;


PROCEDURE Restore_Screen2;
BEGIN
{$IFDEF MSDOS}
     Move(SAVESCRN_Scr2[1],MEM[SegB800:0],4000);
{$ENDIF}
{$IFDEF WIN32}
     RPRestoreScreen(SavedScreen2);
{$ENDIF}
END;


{LOCAL AND REMOTE SCREEN SAVE}


PROCEDURE Save_Screen;
BEGIN
{$IFDEF MSDOS}
     Move(MEM[SegB800:0],SAVESCRN_Scr[1],3840);
{$ENDIF}
{$IFDEF WIN32}
     RPSaveScreen(SavedScreen1);
{$ENDIF}
     SAVESCRN_X := WhereX;
     SAVESCRN_Y := WhereY;
     SAVESCRN_Attr := TextAttr;
END;


PROCEDURE Restore_Screen;
VAR  TheAttr : Byte;
     TheChar : Char;
     NewB    : Byte;
     NewF    : Byte;
     Loop    : Word;
     LastOne : Word;
     Temp    : String;
     LoopY   : Word;
     LoopX   : Word;
     LastF   : Word;
     LastB   : Word;
BEGIN
     _HideCursor;
     ClrPortScr;
     ForeBack(15,0);
     LastF := 15;
     LastB := 0;
     Loop  := 2;
     FOR LoopY := 1 TO 24 DO
     BEGIN
       Temp := '';
       FOR LoopX := 1 TO 80 DO
       BEGIN
         IF LoopX=80 THEN
         BEGIN
{$IFDEF MSDOS}
           TheAttr := SAVESCRN_Scr[Loop];
{$ENDIF}
{$IFDEF WIN32}
           // REETODO Does this work?
           TheAttr := Byte(SavedScreen1[LoopX][LoopY].Attributes);
{$ENDIF}
           IF TheAttr>127 THEN DEC(TheAttr,128);
           NewB := Trunc(TheAttr/16);
           NewF := TheAttr-(NewB*16);
           IF ((NewF<>LastF) OR (NewB<>LastB)) THEN
           BEGIN
             LastF := NewF;
             LastB := NewB;
             ForeBack(NewF,NewB);
           END;
         END
         ELSE
         BEGIN
{$IFDEF MSDOS}
           TheAttr := SAVESCRN_Scr[Loop];
{$ENDIF}
{$IFDEF WIN32}
           // REETODO Does this work?
           TheAttr := Byte(SavedScreen1[LoopX][LoopY].Attributes);
{$ENDIF}
           IF TheAttr>127 THEN DEC(TheAttr,128);
{$IFDEF MSDOS}
           TheChar := Chr(SAVESCRN_Scr[Loop-1]);
{$ENDIF}
{$IFDEF WIN32}
           TheChar := SavedScreen1[LoopX][LoopY].AsciiChar;
{$ENDIF}
           NewB := Trunc(TheAttr/16);
           NewF := TheAttr-(NewB*16);
           IF ((NewF<>LastF) OR (NewB<>LastB)) THEN
           BEGIN
             LastF := NewF;
             LastB := NewB;
             ForeBack(NewF,NewB);
           END;
           WRITECHAR(TheChar);
         END;
         INC(Loop,2);
       END;
       IF LoopY<>24 THEN SendCrLf(1);
     END;
     GotoXYAnsi(SAVESCRN_X,SAVESCRN_Y);
     TextAttr := SAVESCRN_Attr;
     _ShowCursor;
END;


END.