#Include %A_LineFile%\..\FindClick.ahk

#If WinActive("Adobe Flash Player")
F12::
  {
    FindClick( A_LineFile "\..\arrow_big.png", "e")
    FindClick( A_LineFile "\..\arrow_small.png", "e")
  }
#If