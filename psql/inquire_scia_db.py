#!/usr/bin/env python
#***********************************************************************
#*               NETHERLANDS ATHMOSPHERIC DATA CENTER                 **
#***********************************************************************
#*                      All rights reserved                           **
#*          Copyright (c) 2008 KNMI, SRON, on behalf of NIVR          **
#*   For license conditions see http://neonet.knmi.nl/licence.html    **
#*                                                                    **
#*             Royal Netherlands Meteorological Institute             **
#*              Netherlands Institute For Space Research              **
#*            Netherlands Agency for Aerospace Programmes             **
#***********************************************************************
#* Function   : inquire_scia_db
#* Purpose    : do a number of predefined database queries
#* Usage      : inquire_scia_db [InquireOptions] [OutputOptions]
#*    
#*      Inquire Options:
#*       --level=<0|1|2> 
#*         select product level
#*       --name=<filename>
#*         select entries with requested product name (no path!)
#*       --rtime=xh[our]/xd[ay]
#*         select entries with receive time less than x hour or x days,
#*         where "x" is an positive integer
#*       --orbit=<number> or --orbit=<min_num>,<max_num>
#*         select entries with requested (absolute) orbit number or range
#*       --proc[stage]=[N,O,P,..]
#*         select entries with requested ProcStage flags
#*       --soft[Version]=SCIA/x.xx
#*         select entries on software version
#*         - to select on version 6 and up , use SCIA/6.%%
#*         - to select on version 6.0 and up , use SCIA/6.0%
#*       --last[Version]
#*         only show entry with highest software version
#*       --date=yyyy[MM[dd[hh[mm]]]]
#*         select entries with states within the requested time-window
#*         - yyyy: returns entries between yyyy and (yyyy+1)
#*         - yyyyMM: returns entries between yyyyMM and yyyy(MM+1)
#*         - yyyyMMdd: returns entries between yyyyMMdd and yyyyMM(dd+1)
#*         - yyyyMMddhh: returns entries between yyyyMMddhh and yyyyMMdd(hh+1)
#*         - yyyyMMddhhmm: returns entries between yyyyMMddhh
#*                                             and yyyyMMddhh(mm+1)
#*         Where yyyy must be in the range 2002 and up
#*               MM   must be in the range 1 - 12
#*               dd   must be in the range 1 - 31
#*               hh   must be in the range 0 - 23
#*               mm   must be in the range 0 - 59
#*         Note: it is up to you to specify valid dates!
#*       --stateID=id,..
#*         select entries with requested State IDs, range [1,70]
#*       --type=nadir,limb,occultation,monitor
#*         select entries of requested Measurement Categories
#*       --lat=<min>,<max>
#*         select entries within given latitude window, range [-90,90]
#*       --lon=<min>,<max>
#*         select entries within given longitude window, range <-180,180]
#*       
#*      Output Options:
#*       --file [default]: only location and filename of selected entries
#*                         are returned
#*       --noPath: return product name (no path)
#*       --header: return all information contained in the header table
#*       --state : return all information contained in the state table
#*       --tile:   return [cld_ol,fresco,imlm,no2_ol,o3_ol]:
#*
#*       The output rows are sorted on orbit number and software version
#*
#* Project    : NADC
#* Module     : lib/python
#*  
#* $Id$
#*
#* $Name$
#*
#* initial programmer :  Richard M. van Hees (SRON)
#* initial date       :  20051117
#* changelog          :  20080330 rewrite to postgreSQL database (RvH)
#*                       20080616 check on date specification (RvH)
#*                       20090821 print path for option lastVersion (RvH)
#*                       20091208 check range longitude, update documentation
#*                       20100108 fixed bug in date selection (RvH)
#*                       20100609 opion --last: do not check S/W version (RvH)
#*                       20100914 opion --noPath: no call to SQLite - fast (RvH)
#*                       20121120 moved to PostgreSQL v9.2 (RvH)
#*                       20121122 fixed bugs in code handling psql arrays (RvH)
#*                       20121126 fixed state selection bug (RvH)
#*                       20130913 port to Python3 (RvH)
#*                       20140224 fixed bug in option --last (RvH)
#*                       
#**********************************************************************
from __future__ import print_function
from __future__ import division

import sys
import psycopg2

from string  import atoi
from os.path import isfile
from datetime import datetime, timedelta

#++++++++++++++++++++++++++++++++++++++++++++++++++
def version_message():
    print( sys.argv[0] + ' -- SciaDC tools version 5.0' )
    sys.exit(0)

#++++++++++++++++++++++++++++++++++++++++++++++++++
def help_message():
    print( 'Usage ' + sys.argv[0]
           + ' [--rtime=] [--orbit=] [--proc[stage]=] [--soft[Version]=]'
           + ' [--date=] [--stateID=] [--type=] [--lat=] [--lon=]'
           + ' [--file] [--noPath] [--header] [--state] [--tile=]'
           + ' [--last[Version]] [--debug] [--level= --name=]' )
    sys.exit(0)

#+++++++++++++++++++++++++
def get_opts():
    import getopt

    from time   import time, localtime, strftime

    try:
        opts,args = getopt.getopt( sys.argv[1:], 'hV', \
                                   ['level=', 'name=', 'orbit=', 'proc=',
                                    'rtime=', 'procstage=', 'softVersion=', 
                                    'date=', 'stateID=', 'type=', 
                                    'lat=', 'lon=', 'last', 'lastVersion', 
                                    'file', 'noPath', 'header', 'state', 
                                    'tile=', 'debug', 'help', 'version'] )

    except getopt.error, msg:
        print( msg )
        print( 'Use -h for help' )
        sys.exit(1)

    if len(opts) == 0: help_message()

# initialize dictionary with default values
    dict_param = {}
    dict_param['Level'] = None
    dict_param['Orbit'] = None
    dict_param['Name'] = None
    dict_param['ProcStage'] = None
    dict_param['softVersion'] = None
    dict_param['lastVersion'] = False
    dict_param['Date'] = None
    dict_param['StateID'] = None
    dict_param['Type'] = None
    dict_param['Received'] = None
    dict_param['Latitude'] = None
    dict_param['Longitude'] = None
    dict_param['Debug'] = False
    dict_param['Output'] = 'filePath'
    dict_param['Tile'] = None

    for opt, arg in opts[:]:
        if opt in ('-h', '--help'):
            help_message()
        elif opt in ('-V', '--version'):
            version_message()
        elif opt in ( '--last', '--lastVersion'):
            dict_param['lastVersion'] = True
        elif opt == '--debug':
            dict_param['Debug'] = True
        elif opt == '--level':
            if arg != '':
                dict_param['Level'] = arg
            else:
                print( opt + ' expect an argument' )
                sys.exit(1)
        elif opt == '--orbit':
            if arg != '':
                dict_param['Orbit'] = arg
            else:
                print( opt + ' expect an argument' )
                sys.exit(1)
        elif opt == '--name':
            if arg != '':
                dict_param['Name'] = arg
            else:
                print( opt + ' expect an argument' )
                sys.exit(1)
        elif opt in ( '--proc', '--procstage'):
            if arg != '':
                dict_param['ProcStage'] = arg
            else:
                print( opt + ' expect an argument' )
                sys.exit(1)
        elif opt in ( '--soft', '--softVersion'):
            if arg != '':
                dict_param['softVersion'] = arg
            else:
                print( opt + ' expect an argument' )
                sys.exit(1)
        elif opt == '--date':
            if arg != '':
                dict_param['Date'] = arg
                if len( dict_param['Date'] ) < 4:
                    print( 'You should atleast specify a year' )
                    sys.exit(1)
            else:
                print( opt + ' expect an argument' )
                sys.exit(1)
        elif opt == '--stateID':
            if arg != '':
                dict_param['StateID'] = arg
            else:
                print( opt + ' expect an argument' )
                sys.exit(1)
        elif opt == '--type':
            if arg != '':
                dict_param['Type'] = arg
            else:
                print( opt + ' expect an argument' )
                sys.exit(1)
        elif opt == '--rtime':
            if arg != '':
                if arg[-1] == 'h':
                    dd = datetime.utcnow() - timedelta(hours=atoi(arg[0:-1]))
                elif arg[-1] == 'd':
                    dd = datetime.utcnow() - timedelta(days=atoi(arg[0:-1]))
                else:
                    print( opt + ' expects xh or xd' )
                    sys.exit(1)
                dict_param['Received'] = \
                    dd.replace( microsecond=0 ).isoformat( ' ' )
            else:
                print( opt + ' expect an argument' )
                sys.exit(1)
        elif opt == '--lat':
            if arg != '':
                arg_lst = arg.split(',')
                if len(arg_lst) == 2:
                    dict_param['Latitude'] = ( float(arg_lst[0]), \
                                               float(arg_lst[1]) )
                else:
                    dict_param['Latitude'] = ( float(arg_lst[0])-0.1, \
                                               float(arg_lst[0])+0.1 )
            else:
                print( opt + ' expect two arguments' )
                sys.exit(1)
        elif opt == '--lon':
            if arg != '':
                arg_lst = arg.split(',')
                if len(arg_lst) == 2:
                    dict_param['Longitude'] = ( float(arg_lst[0]), \
                                                float(arg_lst[1]) )
                else:
                    dict_param['Longitude'] = ( float(arg_lst[0])-0.1, \
                                                float(arg_lst[0])+0.1 )
                if dict_param['Longitude'][0] > 180.:
                    dict_param['Longitude'][0] -= 360.
                if dict_param['Longitude'][1] > 180.:
                    dict_param['Longitude'][1] -= 360.
            else:
                print( opt + ' expect two arguments' )
                sys.exit(1)
        elif opt == '--file':
            dict_param['Output'] = 'filePath'
        elif opt == '--noPath':
            dict_param['Output'] = 'fileName'
        elif opt == '--header':
            dict_param['Output'] = 'header'
        elif opt == '--state':
            dict_param['Output'] = 'state'
        elif opt == '--tile':
            dict_param['Output'] = 'tile'
            if arg == 'cld_ol':
                dict_param['Tile'] = 'tile_cld_ol'
            elif arg == 'no2_ol':
                dict_param['Tile'] = 'tile_no2_ol'
            elif arg == 'o3_ol':
                dict_param['Tile'] = 'tile_o3_ol'
            elif arg == 'fresco':
                dict_param['Tile'] = 'tile_fresco'
            elif arg == 'imlm':
                dict_param['Tile'] = 'tile_imlm'
            else:
                print( opt, ' expects: cld_ol, fresco, imlm, no2_ol, o3_ol' )
                sys.exit(1)

    return dict_param

#+++++++++++++++++++++++++
def JulianAstro( year, month, day ):
    if month < 3:
        year  = year - 1
        month = month + 12
    julian = int(365.25*year) + int(30.6001*(month+1)) + day + 1720994.5
    tmp = year + month / 100.0 + day / 10000.0
    if tmp >= 1582.1015:
        A = year / 100
        B = 2 - A + A/4
        julian = julian + B
    return julian * 1.0

def JulianSCIA( year, month, day, hour, minu ):
    day += (hour + minu / 60.) / 24.
    return JulianAstro( year, month-1, day-1 ) - JulianAstro( 2000, 0, 0 )

#+++++++++++++++++++++++++
def BuildWhereList( dict_param, table_lst ):

    where_lst = []
#
# selection on the Header table(s)
#
    if dict_param['Name']:
        if dict_param['Name'][0:10] == 'SCI_NL__0P':
	    dict_param['Level'] = '0'
            metaTable = 'meta__0P.'
            stateTable = 'stateinfo.'
        elif dict_param['Name'][0:10] == 'SCI_NL__1P':
	    dict_param['Level'] = '1'
            metaTable = 'meta__1P.'
            stateTable = 'stateinfo.'
        elif dict_param['Name'][0:10] == 'SCI_NL__2P':
	    dict_param['Level'] = '2'
            metaTable = 'meta__2P.'
            stateTable = 'stateinfo.'
        else:
            print( 'Unknown product -> no entry' )
            sys.exit(1)

        mystr = metaTable + 'name=\'' + dict_param['Name'] + '\''
        where_lst.append( mystr )
    elif dict_param['Level']:
        if dict_param['Level'] == '0':
            metaTable = 'meta__0P.'
            stateTable = 'stateinfo.'
        elif dict_param['Level'] == '1':
            metaTable = 'meta__1P.'
            stateTable = 'stateinfo.'
        elif dict_param['Level'] == '2':
            metaTable = 'meta__2P.'
            stateTable = 'stateinfo.'
        else:
            print( 'Unknown product -> no entry' )
            sys.exit(1)
    else:
        print( 'Please select at least on name or product level' )
        sys.exit(1)

# define list of tables
    table_lst.append( metaTable[:-1] )
    table_lst.append( stateTable[:-1] )
#    table_lst.append( tileTable[:-1] )
#
# selections on metaTable
#
    if dict_param['Received']:
        mystr = metaTable \
                 + 'receiveDate>=\'' + dict_param['Received'] + '\''
        where_lst.append( mystr )

    if dict_param['softVersion']:
        where_lst.append( metaTable + 'softVersion like \'' \
                          + dict_param['softVersion'] + '%\'' )
#
# selection on stateTable
#
    if dict_param['StateID']:
        if len( dict_param['StateID'] ) == 1:
            mystr = stateTable + 'stateID=' + dict_param['StateID']
        else:
            mystr = stateTable + 'stateID IN (' + dict_param['StateID'] + ')'
        where_lst.append( mystr )

    if dict_param['Type']:
        mystr = stateTable
        if dict_param['Type'] == 'nadir':
            mystr += 'stateID IN (1,2,3,4,5,6,7,11,12,13,14,15)'
        elif dict_param['Type'] == 'limb':
            mystr += 'stateID IN (28,29,30,31,32,33)'
        elif dict_param['Type'] == 'occultation':
            mystr += 'stateID IN (56,57)'
        elif dict_param['Type'] == 'monitor':
            mystr += 'stateID IN (8,16,26,39,46,48,51,52,59,61,62,63,65,69,70)'
        else:
            print( 'Valid values for type are: ',
                   'nadir, limb, occultation, monitor' )
            sys.exit(1)
        where_lst.append( mystr )

    if dict_param['Output'] == 'tile':
        tableName = tileTable
    else:
        tableName = stateTable
    if dict_param['Longitude'] and dict_param['Latitude']:
        mystr = "%stile && ST_GeomFromText(\'POLYGON((" \
                "%g %g,%g %g,%g %g,%g %g,%g %g))\',4326)" \
                % ( tableName, dict_param['Longitude'][1],
                    dict_param['Latitude'][0],
                    dict_param['Longitude'][1],
                    dict_param['Latitude'][1],
                    dict_param['Longitude'][0],
                    dict_param['Latitude'][1],
                    dict_param['Longitude'][0],
                    dict_param['Latitude'][0],
                    dict_param['Longitude'][1],
                    dict_param['Latitude'][0] )
        where_lst.append( mystr )
    elif dict_param['Longitude']:
        mystr = "%stile && ST_GeomFromText(\'POLYGON((" \
                "%g %g,%g %g,%g %g,%g %g,%g %g))\',4326)" \
                % ( tableName, dict_param['Longitude'][1], -90,
                    dict_param['Longitude'][1], 90,
                    dict_param['Longitude'][0], 90,
                    dict_param['Longitude'][0], -90,
                    dict_param['Longitude'][1], -90 )
        where_lst.append( mystr )
    elif dict_param['Latitude']:
        mystr = "%stile && ST_GeomFromText(\'POLYGON((" \
                "%g %g,%g %g,%g %g,%g %g,%g %g))\',4326)" \
                % ( tableName, 180, dict_param['Latitude'][0],
                    180, dict_param['Latitude'][1],
                    -180, dict_param['Latitude'][1],
                    -180, dict_param['Latitude'][0],
                    180, dict_param['Latitude'][0] )
        where_lst.append( mystr )
#
# selection on metaTable or stateTable
#
    if dict_param['Output'] == 'state':
        tableName = stateTable
    else:
        tableName = metaTable

    if dict_param['ProcStage']:
        if len( dict_param['ProcStage'] ) == 1:
            if tableName == metaTable:
                where_lst.append( metaTable + 'procStage=\'' \
                                  + dict_param['ProcStage'] + '\'' )
            else:
                buff = 'substr(%ssoftVersion,%d,1)=\'%s\'' \
                       % (tableName, int(dict_param['Level']) + 1,
                          dict_param['ProcStage'])
                where_lst.append( buff )
        else:
            if tableName == metaTable:
                mystr = tableName + 'procStage IN ('
                for c in dict_param['ProcStage']:
                    if mystr[-1] != '(':  mystr += ','
                    mystr += '\'' + c + '\''
                mystr += ')'
            else:
                mystr = 'substr(%ssoftVersion,%d,1) IN (' \
                    % (tableName, int(dict_param['Level']) + 1)
                for c in dict_param['ProcStage']:
                    if mystr[-1] != '(':  mystr += ','
                    mystr += '\'' + c + '\''
                mystr += ')'
            where_lst.append( mystr )

    if dict_param['Output'] == 'state':
        tableName = stateTable
    else:
        tableName = metaTable
        for mystr in where_lst:
            if mystr.split('.')[0].strip('(') == 'stateinfo':
                tableName = stateTable
                break

    if dict_param['Orbit']:
        orbitList = dict_param['Orbit'].split(',')
        if len( orbitList ) == 1:
            mystr = tableName + 'absOrbit=' + orbitList[0]
        else:
            mystr = tableName + 'absOrbit BETWEEN '\
                +  orbitList[0] + ' AND ' + orbitList[1]
        where_lst.append( mystr )
        if dict_param['lastVersion']:
            if tableName == metaTable:
                otherTable = stateTable
            else:
                otherTable = metaTable
            if len( orbitList ) == 1:
                mystr = otherTable + 'absOrbit=' + orbitList[0]
            else:
                mystr = otherTable + 'absOrbit BETWEEN '\
                    +  orbitList[0] + ' AND ' + orbitList[1]
            where_lst.append( mystr )

    if dict_param['Date']:
        year = int(dict_param['Date'][0:4])
        if year < 2002:
            print( 'You should specify a valid year' )
            sys.exit(1)
        dtime = 'year'

        if len( dict_param['Date'] ) >= 6:
            month = int(dict_param['Date'][4:6])
            if month < 1 or month > 12:
                print( 'You should specify a valid month' )
                sys.exit(1)
            dtime = 'month'
        else:
            month = 1

        if len( dict_param['Date'] ) >= 8:
            day = int(dict_param['Date'][6:8])
            if day < 1 or day > 31:
                print( 'You should specify a valid day' )
                sys.exit(1)
            dtime = 'day'
        else:
            day = 1
        
        if len( dict_param['Date'] ) >= 10:
            hour = int(dict_param['Date'][8:10])
            if hour < 0 or hour > 24:
                print( 'You should specify a valid hour' )
                sys.exit(1)
            dtime = 'hour'
        else:
            hour = 0
        
        if len( dict_param['Date'] ) >= 12:
            minu = int(dict_param['Date'][10:12])
            if minu < 0 or minu > 59:
                print( 'You should specify a valid minu' )
                sys.exit(1)
            dtime = 'minu'
        else:
            minu = 0
        secnd = 0
        d1 = datetime( year, month, day, hour, minu, secnd )

        if dtime == 'year':
            d2 = datetime( year+1, month, day, hour, minu, secnd )
        elif dtime == 'month':
            month += 1
            if month > 12:
                year += 1
                month = 1
            d2 = datetime( year, month, day, hour, minu, secnd )
        elif dtime == 'day':
            dd = timedelta(days=1)
            d2 = d1 + dd
        elif dtime == 'hour':
            dd = timedelta(hours=1)
            d2 = d1 + dd
        else:
            dd = timedelta(hours=1)
            d2 = d1 + dd

        mystr = tableName + 'dateTimeStart BETWEEN \'' \
            + d1.isoformat(' ') + '\' AND \'' + d2.isoformat(' ') + '\''
        where_lst.append( mystr )
        if dict_param['lastVersion']:
            if tableName == metaTable:
                otherTable = stateTable
            else:
                otherTable = metaTable
            mystr = otherTable + 'dateTimeStart BETWEEN \'' \
                + d1.isoformat(' ') + '\' AND \'' + d2.isoformat(' ') + '\''
            where_lst.append( mystr )

    return where_lst

#+++++++++++++++++++++++++
def QueryFile( table_lst, where_tbl, File=True ):

    # build a list of rows to be shown by the select statement
    if File:
        row_lst = 'name,softVersion'
    else:
        row_lst = '*'

    # combine where-actions
    tbl_meta   = None
    where_opt  = None
    for table in table_lst:
        if table[0:4] == 'meta':
            tbl_meta = table
            where_opt = ' ORDER BY absOrbit,procStage,softVersion'
    
    where_expr = None
    if where_tbl.has_key( 'stateinfo' ):
        tbl_state2meta = 'stateinfo_' + tbl_meta
        where_expr = ' WHERE pk_meta IN' \
                     + ' (SELECT DISTINCT fk_meta FROM ' + tbl_state2meta \
                     + ' WHERE fk_stateinfo &&' \
                     + ' ARRAY(SELECT pk_stateinfo FROM stateinfo WHERE ' \
                     +  where_tbl['stateinfo'] + '))'
    if where_tbl.has_key( tbl_meta ):
        if where_expr:
            where_expr += ' AND ' + where_tbl[tbl_meta]
        else:
            where_expr = ' WHERE ' + where_tbl[tbl_meta]
    #
    # set up SELECT query as:
    # SELECT 'row_lst' FROM 'from_lst' WHERE 'where_expr' 'where_opt'
    #
    query_str = 'SELECT ' + row_lst + ' FROM ' + tbl_meta
    if where_expr: query_str += where_expr
    if where_opt:  query_str += where_opt

    return query_str

#+++++++++++++++++++++++++
def QueryState( table_lst, where_tbl ):

    # build a list of rows to be shown by the select statement
    row_lst = 'stateID,absOrbit,dateTimeStart,muSecStart,dateTimeStop,' \
              'muSecStop,timeLine,orbitPhase,softVersion,obmTemp,' \
              'detTemp,pmdTemp,ST_asText(tile)'

    # combine where-actions and join table if necessary
    tbl_meta   = None
    for table in table_lst:
        if table[0:4] == 'meta':
            tbl_meta = table

    where_expr = None
    if where_tbl.has_key( tbl_meta ):
        tbl_state2meta = 'stateinfo_' + tbl_meta
        where_expr = ' WHERE pk_stateinfo ' \
            + 'IN (SELECT unnest(fk_stateinfo) FROM ' \
            + tbl_meta + ' LEFT JOIN ' + tbl_state2meta + ' ON ' \
            + tbl_meta + '.pk_meta=' + tbl_state2meta \
            + '.fk_meta WHERE ' +  where_tbl[tbl_meta] + ')'
        if where_tbl.has_key( 'stateinfo' ):
            where_expr += ' AND ' + where_tbl['stateinfo']
    elif where_tbl.has_key( 'stateinfo' ):
        level = int(tbl_meta[-2])
        where_expr = ' WHERE ' + where_tbl['stateinfo']
        if find( where_tbl['stateinfo'], 'softVersion' ) == -1:
            where_expr += ' AND substr(softVersion,%d,1) <> \'0\'' \
                          % (level+1)
    else:
        level = int(tbl_meta[-2])
        where_expr = 'substr(stateinfo.softVersion,%d,1) <> \'0\'' % (level+1)
    #
    # set up SELECT query as:
    # SELECT 'row_lst' FROM 'from_lst' WHERE 'where_expr' 'where_opt'
    #
    query_str = 'SELECT ' + row_lst + ' FROM stateinfo'
    if where_expr: query_str += where_expr
    query_str += ' ORDER BY dateTimeStart'

    return query_str

#+++++++++++++++++++++++++
def QueryTile( table_lst, where_tbl, tile_tbl ):

    # combine where-actions
    tbl_meta  = None
    tbl_state = None
    tbl_tile  = None
    tbl_match = None
    for table in table_lst:
        if table[0:4] == 'meta':
            tbl_meta = table
        elif table[0:5] == 'state':
            tbl_state = 'stateinfo'

    row_lst = 'pk_tileinfo,ti.julianDay,fk_stateinfo,ti.integrationTime,'\
              'pixelType,positionESM,satZenithAngle,satAzimuthAngle,'\
              'sunZenithAngle,sunAzimuthAngle,ST_asText(tile)'

    if tile_tbl == 'tile_cld_ol':
        row_lst += ',cloudFraction,cloudTopPress,cloudOpticalDepth,cloudBRDF'
        row_lst += ',surfacePress,surfaceRefl,aerosolIndex'
        tbl_match = 'cld_ol_tileinfo'
    elif tile_tbl == 'tile_no2_ol':
        row_lst += ',verticalColumn,slantColumn,amfGround,amfCloud'
        tbl_match = 'no2_ol_tileinfo'
    elif tile_tbl == 'tile_o3_ol':
        row_lst += ',verticalColumn,slantColumn,amfGround,amfCloud'
        tbl_match = 'o3_ol_tileinfo'
    elif tile_tbl == 'tile_fresco':
        row_lst += ',integrationTime,errorFlag'
        row_lst += ',cloudFraction,cloudTopHeight,cloudTopPressure,cloudAlbedo'
        row_lst += ',surfaceHeight,surfacePressure,surfaceAlbedo'
        tbl_match = 'fresco_tileinfo'
    elif tile_tbl == 'tile_imlm':
        row_lst += ',integrationTime,errorFlag'
        row_lst += ',h2o_column,n2o_column,co_column,cloudFraction'
        row_lst += ',surfaceAlbedo,meanElevation'
        tbl_match = 'imlm_tileinfo'
    elif tile_tbl == 'tile_mcfs':
        row_lst += ',integrationTime,quality,numMerisPixels'
        row_lst += ',cloudFraction,cloudFractionThick'
        tbl_match = 'mcfs_tileinfo'
    #
    # set up SELECT query:
    #
    query_ti_str  = None
    query_var_str = tile_tbl
    where_opt = 'ORDER BY julianDay'
    if where_tbl.has_key( tbl_meta ):
        query_ti_str = 'SELECT pk_meta FROM %s WHERE %s' \
                       % (tbl_meta, where_tbl[tbl_meta])
        query_ti_str = 'SELECT DISTINCT fk_stateinfo FROM stateinfo_%s'\
                       ' WHERE fk_meta in (%s)' % (tbl_meta, query_ti_str)
        query_ti_str = 'SELECT * FROM %s WHERE fk_stateinfo IN (%s)' \
                       % (tbl_tile, query_ti_str)
        query_var_str = 'SELECT pk_meta FROM %s WHERE %s' \
                        % (tbl_meta, where_tbl[tbl_meta])
        query_var_str = 'SELECT * FROM %s WHERE fk_meta IN (%s)' \
                        % (tile_tbl, query_var_str)

    if where_tbl.has_key( tbl_state ):
        query_ti_str = 'SELECT pk_stateinfo FROM %s WHERE %s' \
                       % (tbl_state, where_tbl[tbl_state])
        query_ti_str = 'SELECT * FROM %s WHERE fk_stateinfo IN (%s)' \
                       % (tbl_tile, query_ti_str)
    
    if where_tbl.has_key( tbl_tile ):
        if query_ti_str:
            query_ti_str += ' AND ' + where_tbl[tbl_tile]
        else:
            query_ti_str = 'SELECT * FROM %s WHERE %s' \
                           % (tbl_tile, where_tbl[tbl_tile])
    
    query_str = 'SELECT %s FROM (%s) ti, %s, (%s) var'\
                ' WHERE pk_tileinfo = fk_tileinfo AND pk_tile = fk_tile %s'\
                % (row_lst, query_ti_str, tbl_match, query_var_str, where_opt)

    return query_str

#+++++++++++++++++++++++++
def QueryLast( table_lst, where_tbl ):

    # combine where-actions and join table if necessary
    tbl_meta  = None
    where_opt = None
    for table in table_lst:
        if table[0:4] == 'meta':
            tbl_meta = table
    
    # build a list of rows to be shown by the select statement
    where_expr = None
    if where_tbl.has_key( tbl_meta ):
        tbl_state2meta = 'stateinfo_' + tbl_meta
        where_expr = 'WITH sm AS' \
            + ' (SELECT fk_meta,procStage,unnest(fk_stateinfo)' \
            + ' AS fk_stateinfo FROM ' + tbl_state2meta \
            + ' WHERE fk_meta IN (SELECT pk_meta FROM ' + tbl_meta \
            + ' WHERE ' +  where_tbl[tbl_meta] + '))'
    if where_tbl.has_key( 'stateinfo' ):
        if where_expr:
            where_expr += ', ss AS'
        else:
            where_expr += 'WITH ss AS'
        where_expr += ' (SELECT pk_stateinfo,softVersion FROM stateinfo WHERE '\
            + where_tbl['stateinfo'] + ' AND substr(softVersion,2,1) <> \'0\')'

    if where_expr:
        where_expr += ' SELECT name FROM ' + tbl_meta + ' WHERE pk_meta IN' \
            + ' (SELECT DISTINCT fk_meta FROM sm JOIN ss ON' \
            + ' sm.fk_stateinfo=pk_stateinfo where' \
            + ' sm.procStage::text=substr(softVersion,2,1))'
        where_opt = ' ORDER BY absOrbit'
    else:
        where_expr = 'SELECT MAX(name),MAX(procStage) FROM meta__1P'
        where_opt = ' GROUP BY absOrbit ORDER BY absOrbit'
    #
    return where_expr + where_opt

#+++++++++++++++++++++++++
def connect_nadc_db( db_host=None, db_user=None, db_passwd=None ):
    from os      import getenv

    nadc_config = './nadc.config.xml'
    if not isfile( nadc_config ): 
        nadc_config = getenv('HOME') + '/nadc.config.xml'
        if not isfile( nadc_config ):
            nadc_config = None

    if nadc_config:
        fp = open( nadc_config )
        while 1:
            line = fp.readline()
            if not line: break

            words = line.split('=')
            key = words[0].strip()
            if len(words) > 1: val = words[1].strip()
            if key == 'host':
                db_host = val[1:-1]
            elif key == 'port':
                db_host += ':' + val[1:-1]
            elif key == 'user':
                db_user = val[1:-1]
            elif key == 'passwd':
                db_passwd = val[1:-1]

    try:
        cx = psycopg2.connect( host=db_host, user=db_user,
                      password=db_passwd, database='scia' )
    except psycopg2.InterfaceError:
        print( 'Interface Error: failed to connect as '
               + db_user + ' to ' + db_host + '.scia' )
        print( 'Interface Error: failed to connect' )
        return None
    except psycopg2.DatabaseError:
        print( 'Database Error: failed to connect as '
               + db_user + ' to ' + db_host + '.scia' )
        return None
    except psycopg2.Error:
        print( 'General Error: failed to connect as '
               + db_user + ' to ' + db_host + '.scia' )
        return None
    else:
        if db_user != 'nadc_admin':
            cu = cx.cursor()
            cu.execute( 'SET search_path TO nadc_admin,public' )
            cu.close()
        return cx

#+++++++++++++++++++++++++
def get_sqlite_path( sciafl ):
    import sqlite3

    cx = sqlite3.connect( '/SCIA/share/db/sron_scia.db' )
    cu = cx.cursor()

    if sciafl[0:10] == 'SCI_NL__0P':
        level = '0'
    elif sciafl[0:10] == 'SCI_NL__1P':
        level = '1'
    elif sciafl[0:10] == 'SCI_OL__2P':
        level = '2'
    else:
        print( 'Failed to obtain path for: ' + sciafl )
        return None

    query_str = 'SELECT path,name,compression FROM meta__%sP' \
                ' WHERE name=\'%s\'' % (level, sciafl)
    cu.execute( query_str )
    row = cu.fetchone()
    cu.close()
    cx.close()
    if row == None:
        return None
    else:
        if row[2] == 0:
            return row[0] + '/' + row[1]
        else:
            return row[0] + '/' + row[1] + '.gz'

#+++++++++++++++++++++++++ Main Routine +++++++++++++++++++++++++
def inquire_scia_db( dict_param ):
# ---
# build SELECT-action
    table_lst = []
    where_lst = BuildWhereList( dict_param, table_lst )
    if dict_param['Debug']:
        print( 'where_lst: ', where_lst, '\n' )

# build where-action split over the main tables (state/meta)
    where_tbl = {}
    for mystr in where_lst:
        tbl_name = mystr.split('.')[0].strip('(')
        if len(tbl_name.split('(')) == 2: tbl_name = tbl_name.split('(')[1]
        
        if where_tbl.has_key( tbl_name ):
            mystr += ' AND ' + where_tbl[tbl_name]
        where_tbl[tbl_name] = mystr
    if dict_param['Debug']:
        print( 'where_tbl: ', where_tbl, '\n' )

# build the whole query
    query_str  = None
    query2_str = None
    if dict_param['lastVersion']:
        query_str = QueryLast( table_lst, where_tbl )
    elif dict_param['Output'][0:4] == 'file':
        query_str = QueryFile( table_lst, where_tbl )
    elif dict_param['Output'] == 'header':
        query_str = QueryFile( table_lst, where_tbl, File=False )
    elif dict_param['Output'] == 'state':
        query_str = QueryState( table_lst, where_tbl )
    else:
        if dict_param['Tile'] == 'tileinfo':
            query_str = QueryTileInfo( table_lst, where_tbl )
        else:
            query_str = QueryTile( table_lst, where_tbl, dict_param['Tile'] )

    if not query_str:
        print( ' *** Error: no query string created' )
        sys.exit(0)
    if dict_param['Debug']:
        print( 'query_str: ', query_str )
        if query2_str:
            print( 'query2_str: ', query2_str )
        sys.exit(0)

# finaly, do the actual select-action
    db_host='gemini'
    db_user='nadc_user'
    db_passwd=None
    cx = connect_nadc_db( db_host, db_user, db_passwd )
    if cx == None: sys.exit(1)
    cu = cx.cursor()
    cu.execute( query_str )
    while 1:
        row = cu.fetchone()
        if row == None: break

        if dict_param['lastVersion']:
            if dict_param['Output'] == 'fileName':
                print( row[0].strip() )
            else:
                print( get_sqlite_path( row[0].strip() ) )
        elif dict_param['Output'] == 'fileName':
            print( row[0].strip() )
        elif dict_param['Output'] == 'filePath':
            print( get_sqlite_path( row[0].strip() ) )
        else:
            if not query2_str:
                print( ' '.join(str(x) for x in row) )
            else:
                cuu = cx.cursor()
                cuu.execute( query2_str % row[0] )
                row2 = cuu.fetchone()
                print( ' '.join(str(x) for x in row2) )
                cuu.close()

    cu.close()
    cx.close()

#+++++++++++++++++++++++++
if __name__ == '__main__':
    dict_param = get_opts()
    inquire_scia_db( dict_param )
