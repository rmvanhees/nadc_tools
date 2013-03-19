;-----------------------------------------------------------------------------------------
; NAME: readbinarystructure.pro
;
;
; PURPOSE: reads unformatted binary data from a file and stores it in a structure array
;
;
; CATEGORY: Sciamachy Detector Monitor Facility
;
;
; CALLING SEQUENCE: result = ReadBinaryStructure ( FileName , Structure , COUNT=Count)
;
;
; INPUTS: 
;   FileName: full qualified path to the file
;   Structure: structure declaration to be read
;
;
; KEYWORD PARAMETERS: 
;   Count: total number of records read from the file 
;
;
; OUTPUTS: structure array of type {STRUCTURE}
;
;
; EXAMPLE:
;
;
; DEPENDENCIES: none
;
;
; MODIFICATION HISTORY:
;       Written by: Q.L. Kleipool, SRON, November 2004
;       Modified  :
;
;-----------------------------------------------------------------------------------------


function ReadBinaryStructure , FileName , Structure , COUNT=Count
Count  = long(0)
Result = Structure
if n_elements(Filename) EQ 0 then begin
  print,'Error in ReadBinaryStructure: file name not specified.'
  return,Result
endif

if n_elements(Structure) EQ 0 then begin
  print,'Error in ReadBinaryStructure: structure definition not specified.'
  return,Result
endif

if (size(FileName,/TNAME) NE 'STRING') then begin
  print,'Error in ReadBinaryStructure: file name must be a string.'
  return,Result
endif

if (size(Structure,/TNAME) NE 'STRUCT') then begin
  print,'Error in ReadBinaryStructure: structure declaration must be a structure.'
  return,Result
endif

if file_test(FileName) EQ 0 then begin
  print,'ReadBinaryStructure: file does not exist: ',filename
  return,Result
endif

on_ioerror,failure
get_lun,Unit
openr,Unit,FileName
Data = Structure
readu,Unit,Data
fs    = fstat(Unit)
Count = fs.size/fs.cur_ptr
point_lun,Unit,0
if Count GT 1 then Result = replicate(Structure,Count) 
readu,Unit,Result
close,Unit
free_lun,Unit
return,Result

failure:
print,'error while reading from ->  '+FileName
close,Unit
free_lun,Unit
Count = 0
return,Result
end

;-----
