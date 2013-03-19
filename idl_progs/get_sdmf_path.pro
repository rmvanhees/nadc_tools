FUNCTION GET_SDMF_PATH, SDMF_VERSION=SDMF_VERSION
 compile_opt idl2,logical_predicate,hidden

 path = ''
 IF N_ELEMENTS( SDMF_VERSION ) EQ 0 THEN BEGIN
    SDMF_VERSION = GETENV( 'USE_SDMF_VERSION' )
    IF STRLEN( SDMF_VERSION ) EQ 0 THEN SDMF_VERSION = '3.0'
 ENDIF

 CASE SDMF_VERSION OF 
    '2.4': BEGIN
       IF FILE_TEST( '/array/slot1C/share/SDMF/2.4.1', /DIRECTORY ) THEN $
          path = '/array/slot1C/share/SDMF/2.4.1/' $
       ELSE IF FILE_TEST( '/SCIA/SDMF241', /DIRECTORY ) THEN $
          path = '/SCIA/SDMF241/' $
       ELSE $
          MESSAGE, ' *** Path to SDMF directory could not be determined', /INFO
       END
    '3.0': BEGIN
       IF FILE_TEST( '/array/slot1C/share/SDMF/3.0', /DIRECTORY ) THEN $
          path = '/array/slot1C/share/SDMF/3.0/' $
       ELSE IF FILE_TEST( '/SCIA/SDMF30', /DIRECTORY ) THEN $$
          path = '/SCIA/SDMF30/' $
       ELSE $
          MESSAGE, ' *** Path to SDMF directory could not be determined', /INFO
       END
    '3.1': BEGIN
       IF FILE_TEST( '/array/slot0B/SDMF/3.1', /DIRECTORY ) THEN $
          path = '/array/slot0B/SDMF/3.1/' $
       ELSE IF FILE_TEST( '/SCIA/SDMF31', /DIRECTORY ) THEN $$
          path = '/SCIA/SDMF31/' $
       ELSE $
          MESSAGE, ' *** Path to SDMF directory could not be determined', /INFO
       END
    ELSE: BEGIN
       MESSAGE, '*** Invalid version of SDMF requested:' + SDMF_VERSION, /INFO
       END
 ENDCASE
 RETURN, path
END
