{$I DEFINES.INC}
UNIT BsVideo;


INTERFACE


{$IFDEF WIN32}
USES 
  Windows;


TYPE
  TScreenBuf = Array[1..25, 1..80] of TCharInfo; // REETODO Don't hardcode to 80x25
{$ENDIF}


PROCEDURE _HighBackGrounds(TurnOn: Boolean);
PROCEDURE FastWrite(St: String; Row,Col,Attr: Byte);
PROCEDURE ExtendedVideoMode(NumLines: Byte);
PROCEDURE AppHeader(InString: String);
{$IFDEF WIN32}
PROCEDURE RPRestoreScreen(var AScreenBuf: TScreenBuf);
PROCEDURE RPSaveScreen(var AScreenBuf: TScreenBuf);
PROCEDURE RPSetAttrAt(x, y, attr: Word);
{$ENDIF}


IMPLEMENTATION


USES Dos,FastW1,Crt,Version;


{$IFDEF WIN32}
VAR
  StdOut: THandle;
{$ENDIF}


PROCEDURE _HighBackGrounds(TurnOn: Boolean);
{$IFDEF MSDOS}
VAR  Regs : Registers;
{$ENDIF}
BEGIN
{$IFDEF MSDOS}
     FillChar(Regs,SizeOf(Regs),0);
     IF TurnOn THEN Regs.BL := 0
               ELSE Regs.BL := 1;
     Regs.AX := $1003;
     Intr($10,Regs);
{$ENDIF}
{$IFDEF WIN32}
  // REENOTE Win32 dropped blink in favour of permanent high background, so nothing to do here
{$ENDIF}
END;


{$IFDEF MSDOS}
PROCEDURE SwitchTo28Rows; Assembler;
ASM
   mov ax,1202h
   mov bl,30h
   int 10h
   mov ax,0003h
   int 10h
   mov ax,1111h
   mov bl,00h
   int 10h
END;
{$ENDIF}
{$IFDEF WIN32}
PROCEDURE SwitchTo28Rows;
BEGIN
     WriteLn('REETODO BSVIDEO SwitchTo28Rows'); Halt;
END;
{$ENDIF}


{$IFDEF MSDOS}
PROCEDURE SwitchTo30Rows;
VAR  CrtcReg : Array[1..8] Of Word;
     Offset  : Word;
     I,Data  : Byte;
BEGIN
     CrtcReg[1] := $0c11; CrtcReg[2] := $0d06;
     CrtcReg[3] := $3e07; CrtcReg[4] := $ea10;
     CrtcReg[5] := $8c11; CrtcReg[6] := $df12;
     CrtcReg[7] := $e715; CrtcReg[8] := $0616;
     MemW[$0040:$004c] := 8192;
     Mem[$0040:$0084]  :=29;
     Offset := MemW[$0040:$0063];
     ASM
        cli
     END;
     FOR i := 1 TO 8 DO PortW[Offset] := CrtcReg[i];
     Data := Port[$03cc];
     Data := Data AND $33;
     Data := Data OR $C4;
     Port[$03c2]:=Data;
     ASM
        sti
        mov ah,12h
        mov bl,20h
        int 10h
     END;
END;
{$ENDIF}
{$IFDEF WIN32}
PROCEDURE SwitchTo30Rows;
BEGIN
     WriteLn('REETODO BSVIDEO SwitchTo30Rows'); Halt;
END;
{$ENDIF}


PROCEDURE ExtendedVideoMode(NumLines: Byte);
BEGIN
     CASE NumLines OF
     28:  SwitchTo28Rows;
     30:  SwitchTo30Rows;
     END;
END;


PROCEDURE FastWrite(St: String; Row,Col,Attr: Byte);
BEGIN
     FastW1.FastWrite(St,Row,Col,Attr);
END;


PROCEDURE AppHeader(InString: String);
BEGIN
     TextBackground(0);
     TextColor(7);
     ClrScr;
     FastWrite('ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ',1,1,25);
     FastWrite('¿',1,80,16);
     FastWrite('³                                                                              ',2,1,25);
     FastWrite('³',2,80,16);
     FastWrite('³                                                                              ',3,1,25);
     FastWrite('³',3,80,16);
     FastWrite('³                                                                              ',4,1,25);
     FastWrite('³',4,80,16);
     FastWrite('À',5,1,25);
     FastWrite('ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ',5,2,16);
     FastWrite(InString,2,3,31);
     FastWrite(ShotgunVersion,2,79-LENGTH(ShotgunVersion),31);
     FastWrite(ShotgunCopyright,4,10,27);
     GotoXY(1,7);
END;


{$IFDEF WIN32}
{ REETODO Should detect screen size }
PROCEDURE RPRestoreScreen(var AScreenBuf: TScreenBuf);
VAR
  BufSize    : TCoord;
  WritePos   : TCoord;
  DestRect   : TSmallRect;
BEGIN
  BufSize.X       := 80;
  BufSize.Y       := 25;
  WritePos.X      := 0;
  WritePos.Y      := 0;
  DestRect.Left   := 0;
  DestRect.Top    := 0;
  DestRect.Right  := 79;
  DestRect.Bottom := 24;
  WriteConsoleOutput(StdOut,@AScreenBuf[1][1],BufSize,WritePos,DestRect);
END;


{ REETODO Should detect screen size }
PROCEDURE RPSaveScreen(var AScreenBuf: TScreenBuf);
VAR
  BufSize    : TCoord;
  ReadPos    : TCoord;
  SourceRect : TSmallRect;
BEGIN
  BufSize.X         := 80;
  BufSize.Y         := 25;
  ReadPos.X         := 0;
  ReadPos.Y         := 0;
  SourceRect.Left   := 0;
  SourceRect.Top    := 0;
  SourceRect.Right  := 79;
  SourceRect.Bottom := 24;
  ReadConsoleOutput(StdOut,@AScreenBuf[1][1],BufSize,ReadPos,SourceRect);
end;


PROCEDURE RPSetAttrAt(x, y, attr: Word);
VAR
  NumWritten: Longint;
  WriteCoord: TCoord;
BEGIN
  WriteCoord.X := x;
  WriteCoord.Y := y;
  WriteConsoleOutputAttribute(StdOut, @Attr, 1, WriteCoord, NumWritten);
END;
{$ENDIF}


{$IFDEF WIN32}
BEGIN
  StdOut := GetStdHandle(STD_OUTPUT_HANDLE);
{$ENDIF}
END.