;-------------------------------------------------------------------------------
;+
; NAME:
;       CONVERT_TO_RUNS
;
; PURPOSE:
;       converts a list of integers to sorted runs
;       [1,3,5,7] -> [[1,1],[3,3],[5,5],[7,7]]
;       [1,2,3,4] -> [[1,4]] (shortened!) (minimum of 'min_run' elements)
;
; TODO:
;       optimise: use fixed size array instead of apppend operations
;-
;-------------------------------------------------------------------------------
function convert_to_runs, numbers, min_run=min_run
  compile_opt idl2,hidden

  n_states = n_elements(numbers)

; minimum run length
  if n_elements(min_run) eq 0 then $
     min_run = 2

  if n_states eq 1 then $
     return, [numbers,numbers]

; sort the id's
  sorted = [numbers[sort(numbers)],0]

; compute first order differences
  shifted   = [sorted[0],sorted]
  diffs     = sorted - shifted
    
; where slope != 1 -> clear
  idx0      = where(diffs ne 1, count)
  if count eq 0 then $
     return, strjoin(strtrim(string(sorted),2), ',')
  diffs[idx0] = 0

; compute second order differences (effectively start and end of runs)
  shifted   = [diffs[0],diffs]
  diffs2    = [diffs,0] - shifted
  starts    = where(diffs2 eq +1, startcount)
  ends      = where(diffs2 eq -1, endcount)

  runs = [[-1,-1]]
  last = 0
  for i=0, startcount-1 do begin
     if (starts[i]-1) ne last then begin
        series = transpose(sorted[last:starts[i]-2])
        runs   = [ [runs], [series, series] ]
     endif
     if (ends[i]-starts[i]) gt (min_run-2) then begin
        run    = [sorted[starts[i]-1], sorted[ends[i]-1]]
        runs   = [[runs], [run]]
     endif else begin
        series = transpose(sorted[starts[i]-1:ends[i]-1])
        runs   = [[runs], [series, series]]
     endelse
     last = ends[i]
  endfor
  if last lt n_states then begin
     series = transpose(sorted[last:n_states-1])
     runs   = [[runs], [series, series]]
  endif

  if n_elements(runs) gt 2 then $
     runs = runs[*,1:*]

  return, runs
end
