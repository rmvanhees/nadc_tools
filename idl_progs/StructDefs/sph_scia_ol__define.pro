; definition of the structure to hold a Offline "Specific Product Header"
PRO sph_scia_ol__define
compile_opt idl2,hidden

struct = { sph_scia_ol ,$
           dbserver      : bytarr(6)  ,$
           errorsum      : bytarr(5)  ,$
           descriptor    : bytarr(29) ,$
           decont        : bytarr(42) ,$
           nadir_win_uv0 : bytarr(31) ,$
           nadir_win_uv1 : bytarr(31) ,$
           nadir_win_uv2 : bytarr(31) ,$
           nadir_win_uv3 : bytarr(31) ,$
           nadir_win_uv4 : bytarr(31) ,$
           nadir_win_uv5 : bytarr(31) ,$
           nadir_win_uv6 : bytarr(31) ,$
           nadir_win_uv7 : bytarr(31) ,$
           nadir_win_uv8 : bytarr(31) ,$
           nadir_win_uv9 : bytarr(31) ,$
           nadir_win_ir0 : bytarr(31) ,$
           nadir_win_ir1 : bytarr(31) ,$
           nadir_win_ir2 : bytarr(31) ,$
           nadir_win_ir3 : bytarr(31) ,$
           nadir_win_ir4 : bytarr(31) ,$
           nadir_win_ir5 : bytarr(31) ,$
           limb_win_pth  : bytarr(31) ,$
           limb_win_uv0  : bytarr(31) ,$
           limb_win_uv1  : bytarr(31) ,$
           limb_win_uv2  : bytarr(31) ,$
           limb_win_uv3  : bytarr(31) ,$
           limb_win_uv4  : bytarr(31) ,$
           limb_win_uv5  : bytarr(31) ,$
           limb_win_uv6  : bytarr(31) ,$
           limb_win_uv7  : bytarr(31) ,$
           limb_win_ir0  : bytarr(31) ,$
           limb_win_ir1  : bytarr(31) ,$
           limb_win_ir2  : bytarr(31) ,$
           limb_win_ir3  : bytarr(31) ,$
           limb_win_ir4  : bytarr(31) ,$
           occl_win_pth  : bytarr(31) ,$
           occl_win_uv0  : bytarr(31) ,$
           occl_win_uv1  : bytarr(31) ,$
           occl_win_uv2  : bytarr(31) ,$
           occl_win_uv3  : bytarr(31) ,$
           occl_win_uv4  : bytarr(31) ,$
           occl_win_uv5  : bytarr(31) ,$
           occl_win_uv6  : bytarr(31) ,$
           occl_win_uv7  : bytarr(31) ,$
           occl_win_ir0  : bytarr(31) ,$
           occl_win_ir1  : bytarr(31) ,$
           occl_win_ir2  : bytarr(31) ,$
           occl_win_ir3  : bytarr(31) ,$
           occl_win_ir4  : bytarr(31) ,$
           start_time    : bytarr(!nadc.sciaUTCstring) ,$
           stop_time     : bytarr(!nadc.sciaUTCstring) ,$
           stripline     : 0s  ,$
           slice_pos     : 0s  ,$
           no_slice      : 0us ,$
           no_nadir_win  : 0us ,$
           no_limb_win   : 0us ,$
           no_occl_win   : 0us ,$
           start_lat     : 0.d ,$
           start_lon     : 0.d ,$
           stop_lat      : 0.d ,$
           stop_lon      : 0.d $
         }
END
