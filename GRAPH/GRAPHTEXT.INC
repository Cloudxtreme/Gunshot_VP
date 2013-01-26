{$I DEFAULTFONT.INC}

    const
      MaxChars    = 255;  { Maximum nr. of characters in a file }
      MaxFonts    = 16;   { maximum possible fonts              }
      Prefix_Size = $80;  { prefix size to skip                 }
      Signature   = '+';  { Signature of CHR file               }

      _END_OF_CHAR = 0;
      _DO_SCAN = 1;
      _DRAW = 253;
      _MOVE = 254;

    type
      OpCodes = byte;//REETODO(_END_OF_CHAR, _DO_SCAN, _DRAW, _MOVE);

      TFHeader = packed record
         header_size: smallword;    {* Version 2.0 Header Format        *}
         font_name: array[1..4] of char;
         font_size: smallword;      {* Size in byte of file        *}
         font_major: byte;     {* Driver Version Information    *}
         font_minor: byte;
         min_major: byte;      {* BGI Revision Information         *}
         min_minor: byte;
      end;

      THeader = packed record
        Signature:  char;     { signature byte                        }
        Nr_chars:   smallint;  { number of characters in file          }
        Reserved:   byte;
        First_char: byte;     { first character in file               }
        cdefs :     smallint;  { offset to character definitions       }
        scan_flag:  byte;     { TRUE if char is scanable              }
        org_to_cap: shortint;     { Height from origin to top of capitol  }
        org_to_base:shortint;     { Height from origin to baseline        }
        org_to_dec: shortint;     { Height from origin to bot of decender }
        _reserved: array[1..4] of char;
        Unused: byte;
      end;

      TOffsetTable =array[0..MaxChars] of smallint;
      TWidthTable  =array[0..MaxChars] of byte;

      TFontRec = packed record
        name : string[8];
        header : THeader;        { font header   }
        pheader : TFHeader;      { prefix header }
        offsets : TOffsetTable;
        widths : TWidthTable;
        instrlength: longint;    { length of instr, because instr can }
        instr : pchar;           { contain null characters            }
      end;


      TStroke = packed record
        opcode: byte;
        x: smallint;  { relative x offset character }
        y: smallint;  { relative y offset character }
      end;


      TStrokes = Array[0..1000] of TStroke;

    var
       CurrentXRatio, CurrentYRatio: single; (* was graph_float *)
       Fonts : array[1..maxfonts] of tfontrec;
       Strokes: TStrokes; {* Stroke Data Base           *}

{***************************************************************************}
{                         Internal routines                                 }
{***************************************************************************}

    function Decode(byte1,byte2: char; var x,y: smallint): smallint;
    { This routines decoes a signle word in a font opcode section  }
    { to a stroke record.                                          }
      var
       b1,b2: shortint;
     Begin
       b1:=shortint(byte1);
       b2:=shortint(byte2);
       { Decode the CHR OPCODE }
       Decode:=byte((shortint(b1 and $80) shr 6)+(shortint(b2 and $80) shr 7));
       { Now get the X,Y coordinates        }
       { bit 0..7 only which are considered }
       { signed values.                     }
{ disable range check mode }
{$ifopt R+}
{$define OPT_R_WAS_ON}
{$R-}
{$endif}
       b1:=b1 and $7f;
       b2:=b2 and $7f;
       { Now if the MSB of these values are set }
       { then the value is signed, therefore we }
       { sign extend it...                      }
       if (b1 and $40)<>0 then b1:=b1 or $80;
       if (b2 and $40)<>0 then b2:=b2 or $80;
       x:=smallint(b1);
       y:=smallint(b2);
{ restore previous range check mode }
{$ifdef OPT_R_WAS_ON}
{$R+}
{$endif}
     end;

    procedure GetTextPosition(var xpos,ypos: longint; const TextString: string);
     begin
         if textSettings.Font = DefaultFont then
          begin
           if textSettings.direction=horizdir then
            begin
              case textSettings.horiz of
                   centertext : XPos:=(textwidth(textstring) shr 1);
                   lefttext   : XPos:=0;
                   righttext  : XPos:=textwidth(textstring);
              end;
              case textSettings.vert of
                  centertext : YPos:=-(textheight(textstring) shr 1);
                  bottomtext : YPos:=-textheight(textstring);
                  toptext    : YPos:=0;
              end;
            end else
            begin
              case textSettings.horiz of
                   centertext : XPos:=(textheight(textstring) shr 1);
                   lefttext   : XPos:=textheight(textstring);
                   righttext  : XPos:=textheight(textstring);
              end;
              case textSettings.vert of
                  centertext : YPos:=(textwidth(textstring) shr 1);
                  bottomtext : YPos:=0;
                  toptext    : YPos:=textwidth(textstring);
              end;
            end;
          end
         else
          begin
            if textSettings.direction=horizdir then
            begin
              case textSettings.horiz of
                   centertext : XPos:=(textwidth(textstring) shr 1);
                   lefttext   : XPos:=0;
                   righttext  : XPos:=textwidth(textstring);
              end;
              case textSettings.vert of
                  centertext : YPos:=(textheight(textstring) shr 1);
                  bottomtext : YPos:=0;
                  toptext    : YPos:=textheight(textstring);
              end;
            end else
            begin
              case textSettings.horiz of
                   centertext : XPos:=(textheight(textstring) shr 1);
                   lefttext   : XPos:=0;
                   righttext  : XPos:=textheight(textstring);
              end;
              case textSettings.vert of
                  centertext : YPos:=(textwidth(textstring) shr 1);
                  bottomtext : YPos:=0;
                  toptext    : YPos:=textwidth(textstring);
              end;
           end;
          end;
     end;

    procedure InstallDefaultFonts;
    begin
       InstallUserFont('TRIP');
       InstallUserFont('LITT');
       InstallUserFont('SANS');
       InstallUserFont('GOTH');
       InstallUserFont('SCRI');
       InstallUserFont('SIMP');
       InstallUserFont('TSCR');
       InstallUserFont('LCOM');
       InstallUserFont('EURO');
       InstallUserFont('BOLD');
    end;

    function testfont(p : pchar) : boolean;

      begin
         testfont:=(p[0]='P') and
          (p[1]='K') and
          (p[2]=#8) and
          (p[3]=#8);
      end;

    function unpack(buf: pchar; index: smallint; var Stroke: TStrokes): smallint;
     var
      po: TStrokes;
      num_ops: smallint;
      opcode, i, opc: word;
      counter: smallint;
      lindex: smallint;
      jx, jy: smallint;
     begin
       num_ops := 0;
       counter := index;
       lindex :=0;


       while TRUE do    {* For each byte in buffer      *}
         Begin
           Inc(num_ops);  {* Count the operation                *}
           opcode := decode( buf[counter], buf[counter+1] ,jx, jy );
           Inc(counter,2);
           if( opcode = ord(_END_OF_CHAR) ) then break; {* Exit loop at end of char     *}
         end;

       counter:=index;

       for i:=0 to num_ops-1 do    {    /* For each opcode in buffer    */ }
         Begin
           opc := decode(buf[counter], buf[counter+1], po[lindex].x, po[lindex].y);  {* Decode the data field   *}
           inc(counter,2);
           po[lindex].opcode := opc;      {* Save the opcode            *}
           Inc(lindex);
         end;
       Stroke:=po;
       unpack := num_ops;       {* return OPS count             *}
     end;

{***************************************************************************}
{                         Exported routines                                 }
{***************************************************************************}

    procedure GetTextSettings(var TextInfo : TextSettingsType);

      begin
         textinfo:=textSettings;
      end;

    function InstallUserFont(const FontFileName : string) : smallint;
      begin
         grResult:=grOk;
         { first check if we do not allocate too many fonts! }
         if installedfonts=maxfonts then
           begin
              grResult:=grError;
              InstallUserFont := DefaultFont;
              exit;
           end;
         inc(installedfonts);
         fonts[installedfonts].name:=FontFileName;
         fonts[installedfonts].instr := nil;
         fonts[installedfonts].instrlength := 0;
         InstallUserFont:=installedfonts;
      end;

    procedure OutText(const TextString : string);
      var x,y:smallint;
      begin
         { Save CP }
         x:=actX;
         y:=actY;
         OutTextXY(actX,actY,TextString);
         { If the direction is Horizontal and the justification left }
         { then and only then do we update the CP                    }
         if (textSettings.direction=HorizDir) and
           (textSettings.horiz=LeftText) then
               inc(x,textwidth(TextString));
         { Update the CP }
         actX := X;
         actY := Y;
      end;

    procedure OutTextXY(x,y : smallint;const TextString : string);

      type
       Tpoint = record
         X,Y: smallint;
       end;
      var
         ConvString    : String;
         i,j,k,c       : longint;
         xpos,ypos     : longint;
         counter       : longint;
         cnt1,cnt2     : smallint;
         cnt3,cnt4     : smallint;
         charsize      : word;
         WriteMode     : word;
         curX2, curY2, xpos2, ypos2, x2, y2: single; (* was graph_float *)
         oldvalues     : linesettingstype;
         fontbitmap    : TBitmapChar;
         chr           : char;
         curx2i,cury2i,
         xpos2i,ypos2i : longint;

      begin
         { save current write mode }
(*REETODO         WriteMode := CurrentWriteMode;
         CurrentWriteMode := NormalPut;*)
         GetTextPosition(xpos,ypos,textstring);
         X:=X-XPos; Y:=Y+YPos;
         XPos:=X; YPos:=Y;

         ConvString := TextString; (* was ConvertString(TextString); *)
         CharSize := textSettings.Charsize;
         if textSettings.font=DefaultFont then
         begin
           c:=length(ConvString);
           if textSettings.direction=HorizDir then
           { Horizontal direction }
             begin
                for i:=0 to c-1 do
                  begin
                     xpos:=x+(i*8)*Charsize;
                     { we copy the character bitmap before accessing it }
                     { this improves speed on non optimizing compilers  }
                     { since it is one less address calculation.        }
                     Fontbitmap:=TBitmapChar(DefaultFontData[ConvString[i+1]]);
                     { no scaling }
                     if CharSize = 1 then
                      Begin
                        for j:=0 to 7 do
                           for k:=0 to 7 do
                             if Fontbitmap[j,k]<>0 then
                               PutPixel(xpos+k,j+y,frColor);
(*REETODO                             else if DrawTextBackground then
                               PutPixel(xpos+k,j+y,bkColor);*)
                      end
                     else
                      { perform scaling of bitmap font }
                      Begin
                        j:=0;
                        cnt3:=0;

                        while j <= 7 do
                        begin
                          { X-axis scaling }
                          for cnt4 := 0 to charsize-1 do
                           begin
                             k:=0;
                             cnt2 := 0;
                             while k <= 7  do
                                begin
                                  for cnt1 := 0 to charsize-1 do
                                    begin
                                       If FontBitmap[j,k] <> 0 then
                                           PutPixel(xpos+cnt1+cnt2,y+cnt3+cnt4,frColor);
(*REETODO                                       else if DrawTextBackground then
                                           PutPixel(xpos+cnt1+cnt2,y+cnt3+cnt4,bkColor);*)
                                    end;
                                    Inc(k);
                                    Inc(cnt2,charsize);
                                end;
                           end;
                          Inc(j);
                          Inc(cnt3,charsize);
                        end;
                      end;
                  end;
             end
           else
           { Vertical direction }
            begin
              for i:=0 to c-1 do
              begin

                chr := ConvString[i+1];
                Fontbitmap:=TBitmapChar(DefaultFontData[chr]);
                ypos := y-(i shl 3)*CharSize;

                { no scaling }
                if CharSize = 1 then
                 Begin
                   for j:=0 to 7 do
                      for k:=0 to 7 do
                        if Fontbitmap[j,k] <> 0 then
                          PutPixel(xpos+j,ypos-k,frColor);
(*REETODO                        else if DrawTextBackground then
                          PutPixel(xpos+j,ypos-k,bkColor);*)
                 end
                else
                 { perform scaling of bitmap font }
                 Begin
                   j:=0;
                   cnt3:=0;

                   while j<=7 do
                   begin
                     { X-axis scaling }
                     for cnt4 := 0 to charsize-1 do
                      begin
                        k:=0;
                        cnt2 := 0;
                        while k<=7  do
                           begin
                             for cnt1 := 0 to charsize-1 do
                               begin
                                  If FontBitmap[j,k] <> 0 then
                                    PutPixel(xpos+cnt3-cnt4,ypos+cnt1-cnt2,frColor);
(*REETODO                                  else if DrawTextBackground then
                                    PutPixel(xpos+cnt3-cnt4,ypos+cnt1-cnt2,bkColor);*)
                               end;
                               Inc(k);
                               Inc(cnt2,charsize);
                           end;
                      end;
                     Inc(j);
                     Inc(cnt3,charsize);
                   end;
                 end;
              end;
            end;
         end else
         { This is a stroked font which is already loaded into memory }
           begin
              getlinesettings(oldvalues);
              { reset line style to defaults }
              setlinestyle(solidln,oldvalues.pattern,normwidth);
              if textSettings.direction=vertdir then
                 xpos:=xpos + Textheight(ConvString);
              CurX2:=xpos; xpos2 := curX2; x2 := xpos2;
              CurY2:=ypos; ypos2 := curY2; y2 := ypos2;
{              x:=xpos; y:=ypos;}

              for i:=1 to length(ConvString) do
                begin
                   c:=byte(ConvString[i]);
{                   Stroke_Count[c] := }
                   unpack( fonts[textSettings.font].instr,
                     fonts[textSettings.font].Offsets[c], Strokes );
                   counter:=0;
                   while true do
                     begin
                         if textSettings.direction=VertDir then
                           begin
                             xpos2:=x2-(Strokes[counter].Y*CurrentYRatio);
                             ypos2:=y2-(Strokes[counter].X*CurrentXRatio);
                           end
                         else
                           begin
                             xpos2:=x2+(Strokes[counter].X*CurrentXRatio) ;
                             ypos2:=y2-(Strokes[counter].Y*CurrentYRatio) ;
                           end;
                         case opcodes(Strokes[counter].opcode) of
                           _END_OF_CHAR: break;
                           _DO_SCAN: begin
                                    { Currently unsupported };
                                    end;
                           _MOVE : Begin
                                     CurX2 := XPos2;
                                     CurY2 := YPos2;
                                   end;
                           _DRAW: Begin
                                    curx2i:=trunc(CurX2);
                                    cury2i:=trunc(CurY2);
                                    xpos2i:=trunc(xpos2);
                                    ypos2i:=trunc(ypos2);
                                    { this optimization doesn't matter that much
                                    if (curx2i=xpos2i) then
                                      begin
                                         if (cury2i=ypos2i) then
                                           putpixel(curx2i,cury2i,currentcolor)
                                         else if (cury2i+1=ypos2i) or
                                           (cury2i=ypos2i+1) then
                                            begin
                                               putpixel(curx2i,cury2i,currentcolor);
                                               putpixel(curx2i,ypos2i,currentcolor);
                                            end
                                          else
                                            Line(curx2i,cury2i,xpos2i,ypos2i);
                                      end
                                    else if (cury2i=ypos2i) then
                                      begin
                                         if (curx2i+1=xpos2i) or
                                           (curx2i=xpos2i+1) then
                                            begin
                                               putpixel(curx2i,cury2i,currentcolor);
                                               putpixel(xpos2i,cury2i,currentcolor);
                                            end
                                          else
                                            Line(curx2i,cury2i,xpos2i,ypos2i);
                                      end
                                    else
                                    }
                                    Line(curx2i,cury2i,xpos2i,ypos2i);
                                    CurX2:=xpos2;
                                    CurY2:=ypos2;
                                  end;
                             else
                               Begin
                               end;
                            end;
                        Inc(counter);
                     end; { end while }
                   if textSettings.direction=VertDir then
                     y2:=y2-(byte(fonts[textSettings.font].widths[c])*CurrentXRatio)
                   else
                     x2:=x2+(byte(fonts[textSettings.font].widths[c])*CurrentXRatio);
                end;
              setlinestyle( oldvalues.linestyle, oldvalues.pattern, oldvalues.thickness);
           end;
        { restore write mode }
(*REETODO        CurrentWriteMode := WriteMode;*)
      end;

    function RegisterBGIfont(font : pointer) : smallint;

      var
         hp : pchar;
         b : word;
         i: longint;
         Header: THeader;
         counter: longint;
         FontData: pchar;
         FHeader: TFHeader;
      begin
         RegisterBGIfont:=grInvalidFontNum;
         i:=0;
         { Check if the font header is valid first of all }
         if testfont(font) then
           begin
              hp:=pchar(font);
              { Move to EOF in prefix header }
              while (hp[i] <> chr($1a)) do Inc(i);
              System.move(hp[i+1],FHeader,sizeof(FHeader));
              System.move(hp[Prefix_Size],header,sizeof(Header));
{$ifdef FPC_BIG_ENDIAN}
              swap_fheader(fheader);
              swap_header(header);
{$endif FPC_BIG_ENDIAN}
              { check if the font name is already allocated? }
              i:=Prefix_Size+sizeof(Header);
              for b:=1 to installedfonts do
                begin
                   if fonts[b].name=FHeader.Font_name then
                     begin
                        System.move(FHeader,fonts[b].PHeader,sizeof(FHeader));
                        System.move(Header,fonts[b].Header,sizeof(Header));
                        System.move(hp[i],Fonts[b].Offsets[Fonts[b].Header.First_Char],Fonts[b].Header.Nr_chars*sizeof(smallint));
{$ifdef FPC_BIG_ENDIAN}
                        swap_offsets(Fonts[b].Offsets,Fonts[b].Header.First_Char,Fonts[b].Header.Nr_chars);
{$endif FPC_BIG_ENDIAN}
                        Inc(i,Fonts[b].Header.Nr_chars*sizeof(smallint));
                        System.move(hp[i],Fonts[b].Widths[Fonts[b].Header.First_Char],Fonts[b].Header.Nr_chars*sizeof(byte));
                        Inc(i,Fonts[b].Header.Nr_chars*sizeof(byte));
                        counter:=Fonts[b].PHeader.font_size+PREFIX_SIZE-i;
                        { allocate also space for null }
                        GetMem(FontData,Counter+1);
                        System.move(hp[i],FontData^,Counter);
                        { Null terminate the string }
                        FontData[counter+1] := #0;
                        if fonts[b].header.Signature<> SIGNATURE then
                          begin
                            grResult:=grInvalidFont;
                            System.Freemem(FontData, Counter+1);
                            exit;
                          end;
                        fonts[b].instr:=FontData;
                        fonts[b].instrlength:=Counter+1;
                        RegisterBGIfont:=b;
                     end;
                end;
           end
         else
           RegisterBGIFont:=grInvalidFont;
      end;

    procedure SetTextJustify(horiz,vert : word);

      begin
         if (horiz<0) or (horiz>2) or
            (vert<0) or (vert>2) then
           begin
              grResult:=grError;
              exit;
           end;
         textSettings.horiz:=horiz;
         textSettings.vert:=vert;
      end;

    procedure SetTextStyle(font,direction : word;charsize : word);

      var
         f : file;
         Prefix: array[0..Prefix_Size-1] of char; {* File Prefix Holder         *}
         Length, Current: longint;
         FontData: Pchar;
         hp  : pchar;
         i   : longint;
      begin
//writeln(font,' ',direction,' ',charsize);readkey;
         if font>installedfonts then
           begin
              grResult:=grInvalidFontNum;
              exit;
           end;

         textSettings.font:=font;
         if (direction<>HorizDir) and (direction<>VertDir) then
           direction:=HorizDir;
         textSettings.direction:=direction;
         { According to the Turbo Pascal programmer's reference }
         { maximum charsize for bitmapped font is 10            }
         if (textSettings.Font = DefaultFont) and (Charsize > 10) then
            textSettings.charsize:=10
         else if charsize<1 then
            textSettings.charsize:=1
         else
            textSettings.charsize:=charsize;

         { This is only valid for stroked fonts }
{$ifdef logging}
         LogLn('(org_to_cap - org_to_dec): ' + strf(
                fonts[textSettings.font].header.org_to_cap-
                fonts[textSettings.font].header.org_to_dec));
{$endif logging}
(*REETODO         if (charsize <> usercharsize) then*)
           Case CharSize of
             1: Begin
                  CurrentXRatio := 0.55;
                  CurrentYRatio := 0.55;
                End;
             2: Begin
                  CurrentXRatio := 0.65;
                  CurrentYRatio := 0.65;
                End;
             3: Begin
                  CurrentXRatio := 0.75;
                  CurrentYRatio := 0.75;
                End;
             4: Begin
                  CurrentXRatio := 1.0;
                  CurrentYRatio := 1.0;
                End;
             5: Begin
                  CurrentXRatio := 1.3;
                  CurrentYRatio := 1.3;
                End;
             6: Begin
                  CurrentXRatio := 1.65;
                  CurrentYRatio := 1.65
                End;
             7: Begin
                  CurrentXRatio := 2.0;
                  CurrentYRatio := 2.0;
                End;
             8: Begin
                  CurrentXRatio := 2.5;
                  CurrentYRatio := 2.5;
                End;
             9: Begin
                  CurrentXRatio := 3.0;
                  CurrentYRatio := 3.0;
                End;
             10: Begin
                   CurrentXRatio := 4.0;
                   CurrentYRatio := 4.0;
                 End
           End;
         { if this is a stroked font then load it if not already loaded }
         { into memory...                                               }
         if (font>DefaultFont) and not assigned(fonts[font].instr) then
           begin
              assign(f,fonts[font].name+'.CHR'); (*REETODO don't hardcode the path*)
{$ifopt I+}
{$define IOCHECK_WAS_ON}
{$i-}
{$endif}
              reset(f,1);
{$ifdef IOCHECK_WAS_ON}
{$i+}
{$endif}
              if ioresult<>0 then
                begin
                   grResult:=grFontNotFound;
                   textSettings.font:=DefaultFont;
                   exit;
                end;
              {* Read in the file prefix        *}
              BlockRead(F, Prefix, Prefix_Size);
              hp:=Prefix;
              i:=0;
              while (hp[i] <> chr($1a)) do Inc(i);
              System.move(hp[i+1],fonts[font].PHeader,sizeof(TFHeader));
              (* Read in the Header file  *)
              BlockRead(F,fonts[font].Header,Sizeof(THeader));
{$ifdef FPC_BIG_ENDIAN}
              swap_fheader(fonts[font].PHeader);
              swap_header(fonts[font].Header);
{$endif FPC_BIG_ENDIAN}
              BlockRead(F,Fonts[font].Offsets[Fonts[font].Header.First_Char],Fonts[font].Header.Nr_chars*sizeof(smallint));
{$ifdef FPC_BIG_ENDIAN}
              swap_offsets(Fonts[font].Offsets,Fonts[font].Header.First_Char,Fonts[font].Header.Nr_chars);
{$endif FPC_BIG_ENDIAN}
              {*        Load the character width table into memory.                     *}
              BlockRead(F,Fonts[font].Widths[Fonts[font].Header.First_Char],Fonts[font].Header.Nr_chars*sizeof(byte));
              {*        Determine the length of the stroke database.                    *}
              current := FilePos( f );          {* Current file location        *}
              Seek( f, FileSize(F));            {* Go to the end of the file    *}
              length := FilePos( f );           {* Get the file length          *}
              Seek( f, current);        {* Restore old file location    *}
              {*        Load the stroke database.                                       *}
              { also allocate space for Null character   }
              Getmem(FontData, Length+1);          {* Create space for font data        *}

              BlockRead(F, FontData^, length-current);        {* Load the stroke data   *}
              FontData[length-current+1] := #0;

             if fonts[font].header.Signature<> SIGNATURE then
             begin
                grResult:=grInvalidFont;
                textSettings.font:=DefaultFont;
                System.Freemem(FontData, Length+1);
                exit;
             end;
             fonts[font].instr:=FontData;
             fonts[font].instrLength:=Length+1;

              if not testfont(Prefix) then
                begin
                   grResult:=grInvalidFont;
                   textSettings.font:=DefaultFont;
                   System.Freemem(FontData, Length+1);
                end;
              close(f);
           end;
      end;

    procedure SetUserCharSize(Multx,Divx,Multy,Divy : word);
      begin
         CurrentXRatio := MultX / DivX;
         CurrentYRatio := MultY / DivY;
      end;

    function TextHeight(const TextString : string) : word;

      begin
         if textSettings.font=DefaultFont
            then TextHeight:=8*textSettings.CharSize
            else
              TextHeight:=Trunc((fonts[textSettings.font].header.org_to_cap-
                fonts[textSettings.font].header.org_to_dec) * CurrentYRatio);
      end;

    function TextWidth(const TextString : string) : word;
      var i,x : smallint;
          c   : byte;
          s   : String;
      begin
         x := 0;
         { if this is the default font ... }
         if textSettings.font = Defaultfont then
            TextWidth:=length(TextString)*8*textSettings.CharSize
         { This is a stroked font ... }
            else begin
               s := TextString; (* was ConvertString(TextString);*)
               for i:=1 to length(s) do
                begin
                   c:=byte(s[i]);
{                   dec(c,fonts[textSettings.font].header.first_char);}
                   if (c-fonts[textSettings.font].header.first_char>=
                       fonts[textSettings.font].header.nr_chars) then
                     continue;
                   x:=x+byte(fonts[textSettings.font].widths[c]);
               end;
             TextWidth:=round(x * CurrentXRatio) ;
            end;
      end;

