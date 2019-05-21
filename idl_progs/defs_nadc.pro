PRO DEFS_NADC, DATADIR
  compile_opt idl2,hidden

  struct = { defs_nadc,      $
             sciaNadir       :      1us ,$
             sciaLimb        :      2us ,$
             sciaOccult      :      3us ,$
             sciaMonitor     :      4us ,$
             maxCluster      :     64us ,$
             numberIrPMD     :      2us ,$
             numberPMD       :      7us ,$
             irChannels      :      3us ,$
             scienceChannels :      8us ,$
             channelSize     :   1024us ,$
             sciencePixels   :   8192us ,$
             numLv0PMDpacket :    200us ,$
             numLv0AuxBCP    :     16us ,$
             numLv0AuxPMTC   :      5us ,$
             sciaUTCstring   :     28us ,$
             dataDir         :  DATADIR $
         }

  DEFSYSV, '!FALSE', (0 EQ 1), 1
  DEFSYSV, '!TRUE', (1 EQ 1), 1
  DEFSYSV, '!nadc', struct, 1
  RETURN
END
