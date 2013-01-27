{$I DEFINES.INC}
UNIT BsMouse;


INTERFACE


USES Dos;


VAR 
{$IFDEF MSDOS}
    Mouse_Reg       : Registers;
{$ENDIF}
    Mouse_Installed : Boolean;
    Mouse_Error     : Word;


PROCEDURE _SetPixeltoMickey(Horiz,Verti : Word);
FUNCTION  _MousePosition (VAR MouseX,MouseY: Word) : Word;
FUNCTION  IsMouseInstalled: Boolean;
FUNCTION  _InitMouse: Word;
PROCEDURE _ShowMouse;
PROCEDURE _HideMouse;


IMPLEMENTATION


{$IFDEF WIN32}
USES
  VPSysLow, Windows;

VAR
  _MouseX, _MouseY: Word;
  StdIn: THandle;
{$ENDIF}


FUNCTION _InitMouse : Word;
BEGIN
{$IFDEF MSDOS}
     WITH Mouse_Reg DO Ax := 0;
     Intr($33,Mouse_Reg);
     _InitMouse := Mouse_Reg.Ax;
{$ENDIF}
{$IFDEF WIN32}
     // Enable the mouse in the console
     _ShowMouse;
  
     // Default mouse position
     _MouseX := 1;
     _MouseY := 1;

     // 65535 seems to be the result value for success
     _InitMouse := 65535;
{$ENDIF}
END;


PROCEDURE _ShowMouse;
{$IFDEF WIN32}
VAR
  Mode: LongInt;
{$ENDIF}
BEGIN
     IF NOT(Mouse_Installed) THEN Exit;
{$IFDEF MSDOS}     
     Mouse_Reg.Ax := 1;
     Intr($33,Mouse_Reg);
{$ENDIF}
{$IFDEF WIN32}
     // Tell the console that we want mouse events
     if (GetConsoleMode(StdIn, Mode)) then
     begin
       Mode := Mode OR ENABLE_MOUSE_INPUT;
       SetConsoleMode(StdIn, Mode);
     end;
{$ENDIF}     
END;


PROCEDURE _HideMouse;
{$IFDEF WIN32}
VAR
  Mode: LongInt;
{$ENDIF}
BEGIN
     IF NOT(Mouse_Installed) THEN Exit;
{$IFDEF MSDOS}
     Mouse_Reg.AX := 2;
     Intr($33,Mouse_Reg);
{$ENDIF}
{$IFDEF WIN32}
     // Tell the console that we don't want mouse events
     if (GetConsoleMode(StdIn, Mode)) then
     begin
       Mode := Mode AND NOT ENABLE_MOUSE_INPUT;
       SetConsoleMode(StdIn, Mode);
     end;
{$ENDIF}
END;


FUNCTION _MousePosition(VAR MouseX,MouseY : Word) : Word;
{$IFDEF WIN32}
VAR
  MouseEvent: TSysMouseEvent;
{$ENDIF}
BEGIN
     IF NOT(Mouse_Installed) THEN
     BEGIN
          MouseX := 1;
          MouseY := 1;
          _MousePosition := 0;
          Exit;
     END;
{$IFDEF MSDOS}
     Mouse_Reg.Ax := 3;
     Intr($33,Mouse_Reg);
     WITH Mouse_Reg DO
     BEGIN
          MouseX := TRUNC((Succ(Cx)+7)/8);
          MouseY := TRUNC((Succ(Dx)+7)/8);
          _MousePosition := Bx;
     END;
{$ENDIF}
{$IFDEF WIN32}
     // Try to get a new mouse event
     if (SysTVGetMouseEvent(MouseEvent)) then
     begin
       // We have an event to handle
       _MouseX := MouseEvent.smePos.X + 1;
       _MouseY := MouseEvent.smePos.Y + 1;
       _MousePosition := MouseEvent.smeButtons;
     end else
     begin
       // No event waiting
       _MousePosition := 0;
     end;
     MouseX := _MouseX;
     MouseY := _MouseY;
{$ENDIF}
END;


FUNCTION IsMouseInstalled: Boolean;
{$IFDEF MSDOS}
VAR  Reg : Registers;
{$ENDIF}
BEGIN
{$IFDEF MSDOS}
     FillChar(Reg,SizeOf(Reg),0);
     WITH Reg DO Ax := 0;
     Intr($33,Reg);
     IsMouseInstalled := Reg.Ax = 65535;
{$ENDIF}
{$IFDEF WIN32}
     // REENOTE Probably a safe assumption
     IsMouseInstalled := True;
{$ENDIF}
END;


PROCEDURE _SetPixeltoMickey(Horiz,Verti : Word);
{$IFDEF MSDOS}
VAR  Regs : Registers;
{$ENDIF}
BEGIN
{$IFDEF MSDOS}
     FillChar(Regs,SizeOf(Regs),0);
     Regs.Ax := 15;
     Regs.Cx := Horiz;
     Regs.Dx := Verti;
     Intr($33,Regs);
{$ENDIF}
{$IFDEF WIN32}
     // REETODO sets the ratio between physical cursor movement (mickeys) and screen coordinate changes
{$ENDIF}
END;


BEGIN
{$IFDEF WIN32}
     StdIn := GetStdHandle(STD_INPUT_HANDLE);
{$ENDIF}
     Mouse_Error := _InitMouse;
     Mouse_Installed := Mouse_Error = 65535;
END.