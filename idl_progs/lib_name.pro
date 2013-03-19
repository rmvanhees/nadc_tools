;$Id: lib_name.pro,v 1.2 2000/06/22 18:17:48 ali Exp $
FUNCTION LIB_NAME, name_arg
  compile_opt idl2,hidden

  name = name_arg
  ON_ERROR, 2
  prefix = ''
  CASE !VERSION.OS_FAMILY OF 
     'unix'  :BEGIN
        CASE !VERSION.OS OF
           'hp-ux': begin
              lib_ext = 'sl'
                                ;HP-UX 10.20 won't find a shared lib
                                ; using LD_LIBRARY_PATH
                                ; probably a misconfiguration of our system
              prefix = getenv( 'LD_LIBRARY_PATH' ) + '/'
           end
           'AIX': begin
                                ;AIX won't find a shared lib in the current dir
                                ; unless the name is preceded with a ./
              lib_ext = 'a'
              prefix = './'
           end
           'sunos': begin
              lib_ext = 'so'
              if (!version.memory_bits eq 64) then name = name + '_64'
           end
           else: lib_ext = 'so'
        ENDCASE 
     END
     'vms'   :  lib_ext = 'EXE'
     'Windows' : lib_ext = 'DLL'
     'MacOS' : lib_ext = 'shlb'
     ELSE: MESSAGE,"Don't know what to do with: " + !VERSION.OS_FAMILY
  ENDCASE

  RETURN, prefix + name + '.' + lib_ext
END
