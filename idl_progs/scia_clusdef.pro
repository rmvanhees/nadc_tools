;
; COPYRIGHT (c) 2013 SRON (R.M.van.Hees@sron.nl)
;
;   This is free software; you can redistribute it and/or modify it
;   under the terms of the GNU General Public License, version 2, as
;   published by the Free Software Foundation.
;
;   The software is distributed in the hope that it will be useful, but
;   WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
;   General Public License for more details.
;
;   You should have received a copy of the GNU General Public License
;   along with this program; if not, write to the Free Software
;   Foundation, Inc., 59 Temple Place - Suite 330, 
;   Boston, MA  02111-1307, USA.
;
;+
; NAME:
;	ROUTINE_NAME
;
; PURPOSE:
;	This function (or procedure) ...
;
; CATEGORY:
;	
;
; CALLING SEQUENCE:
;	ROUTINE_NAME, Parameter1, Parameter2, Foobar
;
;	Note that the routine name is ALL CAPS and arguments have Initial
;	Caps.  For functions, use the form:
; 
;	Result = FUNCTION_NAME(Parameter1, Parameter2, Foobar)
;
;	Always use the "Result = " part to begin. This makes it super-obvious
;	to the user that this routine is a function!
;
; INPUTS:
;	Parm1:	Describe the positional input parameters here. Note again
;		that positional parameters are shown with Initial Caps.
;
; OPTIONAL INPUTS:
;	Parm2:	Describe optional inputs here. If you don't have any, just
;		delete this section.
;	
; KEYWORD PARAMETERS:
;	KEY1:	Document keyword parameters like this. Note that the keyword
;		is shown in ALL CAPS!
;
;	KEY2:	Yet another keyword. Try to use the active, present tense
;		when describing your keywords.  For example, if this keyword
;		is just a set or unset flag, say something like:
;		"Set this keyword to use foobar subfloatation. The default
;		 is foobar superfloatation."
;
; OUTPUTS:
;	Describe any outputs here.  For example, "This function returns the
;	foobar superflimpt version of the input array."  This is where you
;	should also document the return value for functions.
;
; OPTIONAL OUTPUTS:
;	Describe optional outputs here.  If the routine doesn't have any, 
;	just delete this section.
;
; COMMON BLOCKS:
;	BLOCK1:	Describe any common blocks here. If there are no COMMON
;		blocks, just delete this entry.
;
; SIDE EFFECTS:
;	Describe "side effects" here.  There aren't any?  Well, just delete
;	this entry.
;
; RESTRICTIONS:
;	Describe any "restrictions" here.  Delete this section if there are
;	no important restrictions.
;
; PROCEDURE:
;	You can describe the foobar superfloatation method being used here.
;	You might not need this section for your routine.
;
; EXAMPLE:
;	Please provide a simple example here. An example from the PICKFILE
;	documentation is shown below. Please try to include examples that
;       do not rely on variables or data files that are not defined in
;       the example code. Your example should execute properly if typed
;       in at the IDL command line with no other preparation.
;
;	Create a PICKFILE widget that lets users select only files with 
;	the extensions 'pro' and 'dat'.  Use the 'Select File to Read' title 
;	and store the name of the selected file in the variable F.  Enter:
;
;		F = PICKFILE(/READ, FILTER = ['pro', 'dat'])
;
; MODIFICATION HISTORY:
; 	Written by:	
;-
;---------------------------------------------------------------------------
PRO _SET_SCIA_CLUSDEF, state_id, orbit
 compile_opt idl2,logical_predicate,hidden

 IF FILE_TEST( './nadc_clusDef.h5', /READ, /NOEXPAND ) THEN $
    CLUSDEF_DB = './nadc_clusDef.h5' $
 ELSE BEGIN
    DEFSYSV, '!nadc', exists=i
    IF i NE 1 THEN DEFS_NADC, '/SCIA/share/nadc_tools'
    IF !nadc.dataDir EQ '' THEN $
       CLUSDEF_DB = '/SCIA/share/nadc_tools/nadc_clusDef.h5' $
    ELSE $
       CLUSDEF_DB = !nadc.dataDir + '/nadc_clusDef.h5'
 ENDELSE
 IF (~ FILE_TEST( CLUSDEF_DB, /READ, /NOEXPAND )) THEN BEGIN
    MESSAGE, ' Can not read from database: ' + CLUSDEF_DB, /INFO
    RETURN
  ENDIF

  fid = H5F_OPEN( CLUSDEF_DB )
  IF fid LT 0 THEN MESSAGE, 'Could not open database: ' + CLUSDEF_DB

  grpName = 'State_' + STRING(state_id, format='(I02)')
  gid = H5G_OPEN( fid, grpName )
  IF gid LT 0 THEN MESSAGE, 'Could not open group: ' + grpName

  dd = H5D_OPEN( gid, 'metaTable' )
  IF dd LT 0 THEN MESSAGE, 'Could not open dataset: metaTable'
  mtbl = H5D_READ( dd )
  H5D_CLOSE, dd
  IF mtbl[orbit].indx_CLcon EQ 255B THEN BEGIN
     indx = WHERE( mtbl.indx_Clcon NE 255B )
     orbitList = mtbl[indx].orbit
     diff = MIN( ABS(orbitList - orbit), ii )
     mtbl = mtbl[indx[ii]]
  ENDIF ELSE $
     mtbl = mtbl[orbit]

  dd = H5D_OPEN( gid, 'clusDef' )
  IF dd LT 0 THEN MESSAGE, 'Could not open dataset: clusDef'
  space_id = H5D_GET_SPACE( dd )
  mem_space_id = H5S_CREATE_SIMPLE( [64,1] )
  H5S_SELECT_HYPERSLAB, space_id, [0,mtbl.indx_CLcon], [64,1], /reset
  clusDef = H5D_READ( dd, FILE_SPACE=space_id, MEMORY_SPACE=mem_space_id  )
  H5S_CLOSE, mem_space_id
  H5S_CLOSE, space_id
  H5D_CLOSE, dd

  H5G_CLOSE, gid
  H5F_CLOSE, fid

  DEFSYSV, '!scia', {state_id: state_id, orbit: mtbl.orbit, $
                     duration: mtbl.duration, num_clus: mtbl.num_clus, $
                     clusDef: clusDef}
 RETURN
END

;---------------------------------------------------------------------------
PRO SCIA_CLUSDEF, state_id, orbit
 compile_opt idl2,logical_predicate 

 DEFSYSV, '!scia', EXIST=i
 IF i EQ 1 THEN BEGIN
    IF !scia.state_id NE state_id OR !scia.orbit NE orbit THEN BEGIN
       _SET_SCIA_CLUSDEF, state_id, orbit
    ENDIF
 ENDIF ELSE BEGIN
    _SET_SCIA_CLUSDEF, state_id, orbit
 ENDELSE

 RETURN
END
