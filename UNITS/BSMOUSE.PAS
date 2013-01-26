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


FUNCTION _InitMouse : Word;
BEGIN
{$IFDEF MSDOS}
     WITH Mouse_Reg DO Ax := 0;
     Intr($33,Mouse_Reg);
     _InitMouse := Mouse_Reg.Ax;
{$ENDIF}
{$IFDEF WIN32}
     WriteLn('REETODO BSMOUSE _InitMouse'); Halt;
{$ENDIF}
END;


PROCEDURE _ShowMouse;
BEGIN
     IF NOT(Mouse_Installed) THEN Exit;
{$IFDEF MSDOS}     
     Mouse_Reg.Ax := 1;
     Intr($33,Mouse_Reg);
{$ENDIF}
{$IFDEF WIN32}
     WriteLn('REETODO BSMOUSE _ShowMouse'); Halt;
{$ENDIF}     
END;


PROCEDURE _HideMouse;
BEGIN
     IF NOT(Mouse_Installed) THEN Exit;
{$IFDEF MSDOS}
     Mouse_Reg.AX := 2;
     Intr($33,Mouse_Reg);
{$ENDIF}
{$IFDEF WIN32}
     WriteLn('REETODO BSMOUSE _HideMouse'); Halt;
{$ENDIF}
END;


FUNCTION _MousePosition(VAR MouseX,MouseY : Word) : Word;
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
     WriteLn('REETODO BSMOUSE _MousePosition'); Halt;
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
     WriteLn('REETODO BSMOUSE IsMouseInstalled'); Halt;
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
     WriteLn('REETODO BSMOUSE _SetPixeltoMickey'); Halt;
{$ENDIF}
END;


BEGIN
     Mouse_Error := _InitMouse;
     Mouse_Installed := Mouse_Error = 65535;
END.