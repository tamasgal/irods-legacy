myTestRule {
   msiExecCmd("touch", *File, "null","null","null",*Result);
   writeLine("serverLog", "second test touched file *File");
   writeLine("stdout", "second test touched file *File");
   acRunWorkFlow("/raja8/home/rods/msso/mssop1/mssop1.run",*R_BUF);
   msiBytesBufToStr(*R_BUF, *R_STR);
   writeLine("stdout", "NEW: *R_STR");
} 
INPUT 
