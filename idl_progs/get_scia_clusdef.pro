FUNCTION GET_SCIA_CLUSDEF, stateID, orbit=orbit
  compile_opt idl2,logical_predicate,hidden

  struct = { ClusterDef   ,$
             chanID : 0b  ,$
             clusID : 0b  ,$
             start  : 0us ,$
             length : 0us $
           }

  ClusDefIndx = [ 3, 3, 3, 3, 3, 3, 3, 1, 3, 3, $
                  3, 3, 3, 3, 3, 1, 1, 1, 1, 1, $
                  1, 1, 3, 3, 3, 1, 1, 1, 1, 1, $
                  1, 1, 1, 1, 1, 1, 1, 3, 1, 1, $
                  1, 3, 3, 3, 3, 1, 1, 1, 1, 1, $
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, $
                  1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ]
                
  IF stateID LT 1 OR stateID GT N_ELEMENTS(ClusDefIndx) THEN BEGIN
     MESSAGE, 'cluster definition not defined for requested stateID', /INFO
     RETURN, {}
  ENDIF

  ; Check if state definition is available
  IF N_ELEMENTS( orbit ) GT 0 THEN BEGIN
     CASE stateID OF
        8:  IF orbit[0] LT 4151 THEN return, {}
        16: IF orbit[0] LT 1572 THEN return, {}
        26: IF orbit[0] LT 4151 THEN return, {}
        39: IF orbit[0] LT 1990 THEN return, {}
        46: IF orbit[0] LT 1572 THEN return, {}
        48: IF orbit[0] LT 1990 THEN return, {}
        52: IF orbit[0] LT 1572 THEN return, {}
        59: IF orbit[0] LT 1572 THEN return, {}
        61: IF orbit[0] LT 1572 THEN return, {}
        62: IF orbit[0] LT 1572 THEN return, {}
        63: IF orbit[0] LT 1990 THEN return, {}
        65: IF orbit[0] LT 1572 THEN return, {}
        67: IF orbit[0] LT 1990 THEN return, {}
        69: IF orbit[0] LT 1572 THEN return, {}
        70: IF orbit[0] LT 1990 THEN return, {}
        ELSE: IF orbit[0] LT 200 THEN return, {}
     ENDCASE
  ENDIF

  CASE ClusDefIndx[stateID-1] OF
     1: BEGIN
        clusdef = REPLICATE({ClusterDef}, 40)
        clusdef.chanID = [1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, $
                          4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, $
                          7, 7, 7, 7, 7, 8, 8, 8]
        clusdef.clusID = [0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, $
                          0, 1, 2, 3, 4, 0, 1, 2, 3, 4, 0, 1, 2, 3, 4, $
                          0, 1, 2, 3, 4, 0, 1, 2]
        clusdef.start = [0, 5, 197, 552, 842, 1019, 1024, 1029, 1100, 1878, $
                         1972, 2043, 2048, 2058, 2081, 2978, 3067, 3072, 3077, $
                         3082, 3991, 4091, 4096, 4101, 4106, 5097, 5115, 5120, $
                         5130, 5144, 6117, 6134, 6144, 6154, 6192, 7132, 7158, $
                         7168, 7178, 8182]
        clusdef.length = [5, 192, 355, 290, 177, 5, 5, 71, 778, 94, 71, 5, 10, $
                          23, 897, 89, 5, 5, 5, 909, 100, 5, 5, 5, 991, 18, 5, $
                          10, 14,973, 17, 10, 10, 38, 940, 26, 10, 10, 1004, 10]
     END
     3: BEGIN
        clusdef = REPLICATE({ClusterDef}, 56)
        clusdef.chanID = [1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, $
                          3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, $
                          5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, $
                          6, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8]
        clusdef.clusID = [0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, $
                          0, 1, 2, 3, 4, 5, 6, 7, 8, $
                          0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, $
                          0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, $
                          0, 1, 2, 3, 4, 5, 0, 1, 2]
        clusdef.start = [0, 5, 197, 552, 748, 1019, 1024, 1100, 1214, 1878, $
                         2043, 2048, 2081, 2131, 2211, 2647, 2722, 2809, 2944, $
                         3067, 3072, 3082, 3118, 3150, 3685, 3819, 3925, 4091, $
                         4096, 4106, 4152, 4180, 4705, 4863, 5115, 5120, 5144, $
                         5227, 5455, 5481, 5659, 5687, 5866, 6020, 6051, 6065, $
                         6134, 6144, 6192, 6437, 6585, 7027, 7158, 7168, 7178, $
                         8182]
        clusdef.length = [5, 192, 355, 196, 94, 5, 5, 114, 664, 94, 5, 10, 50, $
                          80, 436, 75, 87, 135, 34, 5, 5, 36, 32, 535, 134, $
                          106, 66, 5, 5, 46, 28, 525, 158, 234, 5, 10, 83, $
                          228, 26, 178, 28, 179, 154, 31, 14, 52, 10, 10, $
                          245, 148, 442, 105, 10, 10, 1004, 10]
     END
  ENDCASE

  RETURN, clusdef
END
