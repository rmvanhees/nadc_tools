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
#* Function   : inquire_gome_db
#* Purpose    : do a number of predefined database queries
#* Usage      : inquire_gome_db [InquireOptions] [OutputOptions]
#*    
#*      Inquire Options:
#*       --level=<1|2> 
#*         select product level
#*       --name=<filename>
#*         select entries with requested product name (no path!)
#*       --rtime=xh[our]/xd[ay]
#*         select entries with receive time less than x hour or x days,
#*         where "x" is an positive integer
#*       --orbit=<number> or --orbit=<min_num>,<max_num>
#*         select entries with requested (absolute) orbit number or range
#*       --soft[Version]=x.xx
#*         select entries on software version
#*       --lastVersion
#*         only show entry with highest software version
#*       --date=yyyy[MM[dd[hh[mm]]]]
#*         select entries with states within the requested time-window
#*         - yyyy: returns entries between yyyy and (yyyy+1)
#*         - yyyyMM: returns entries between yyyyMM and yyyy(MM+1)
#*         - yyyyMMdd: returns entries between yyyyMMdd and yyyyMM(dd+1)
#*         - yyyyMMddhh: returns entries between yyyyMMddhh and yyyyMMdd(hh+1)
#*         - yyyyMMddhhmm: returns entries between yyyyMMddhh
#*                                             and yyyyMMddhh(mm+1)
#*         Where yyyy must be in the range 1995 and up
#*               MM   must be in the range 1 - 12
#*               dd   must be in the range 1 - 31
#*               hh   must be in the range 0 - 23
#*               mm   must be in the range 0 - 59
#*         Note: it is up to you to specify valid dates!
#*       --lat=<min>,<max>
#*         select entries within given latitude window, range [-90,90]
#*       --lon=<min>,<max>
#*         select entries within given longitude window, range [-180,180]
#*       
#*      Output Options:
#*       --link: show link to file instead of real path
#*       --file [default]: only location and filename of selected entries
#*                         are returned
#*       --header: all information contained in the header table is returned
#*       --tile=[geo,pmd,gdp]:
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
#* initial date       :  20080520
#*                       20080616 check on date specification (RvH)
#*                       20090811 support new filenames of GOME level 1 (RvH)
#*                       20100810 support new filenames of GOME level 1 (RvH)
#*                       20130913 port to Python3 (RvH)
#*     
#**********************************************************************

from __future__ import print_function
from __future__ import division

import sys
import psycopg2

from string  import atoi
from os.path import isfile

#++++++++++++++++++++++++++++++++++++++++++++++++++
def version_message():
    print( sys.argv[0] + ' -- SciaDC tools version 5.0' )
    sys.exit(0)

#++++++++++++++++++++++++++++++++++++++++++++++++++
def help_message():
    print( 'Usage ' + sys.argv[0]
           + ' [--debug] [--rtime=] [--orbit=] [--soft[Version]=] [--date=]'
           + ' [--lat=] [--lon=] [--lastVersion] [--link]'
           + ' [--file --header --tile=] [--level= --name=]' )
    sys.exit(0)

#+++++++++++++++++++++++++
def get_opts():
    import getopt

    from time     import time, localtime, strftime
    from datetime import datetime, timedelta

    try:
        opts,args = getopt.getopt( sys.argv[1:], 'hV', \
                                   ['level=', 'name=', 'orbit=', 'date=',
                                    'softVersion=', 'lastVersion',
                                    'rtime=', 'lat=', 'lon=', 
                                    'file', 'link', 'header', 'tile=', 
                                    'debug', 'help', 'version'] )
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
    dict_param['softVersion'] = None
    dict_param['lastVersion'] = False
    dict_param['Date'] = None
    dict_param['Received'] = None
    dict_param['Latitude'] = None
    dict_param['Longitude'] = None
    dict_param['Debug'] = False
    dict_param['Link'] = False
    dict_param['Output'] = 'file'
    dict_param['Tile'] = None

    for opt, arg in opts[:]:
        if opt in ('-h', '--help'):
            help_message()
        elif opt in ('-V', '--version'):
            version_message()
        elif opt == '--lastVersion':
            dict_param['lastVersion'] = True
        elif opt == '--debug':
            dict_param['Debug'] = True
        elif opt == '--link':
            dict_param['Link'] = True
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
        elif opt in ( '--soft', '--softVersion'):
            if arg != '':
                dict_param['softVersion'] = arg
            else:
                print( opt + ' expect an argument' )
                sys.exit(1)
        elif opt == '--date':
            if arg != '':
                dict_param['Date'] = arg
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
                arg_lst = arg.splot(',')
                if len(arg_lst) == 2:
                    dict_param['Longitude'] = ( float(arg_lst[0]), \
                                                float(arg_lst[1]) )
                else:
                    dict_param['Longitude'] = ( float(arg_lst[0])-0.1, \
                                                float(arg_lst[0])+0.1 )
            else:
                print( opt + ' expect two arguments' )
                sys.exit(1)
        elif opt == '--file':
            dict_param['Output'] = 'file'
        elif opt == '--header':
            dict_param['Output'] = 'header'
        elif opt == '--tile':
            dict_param['Output'] = 'tile'
            if arg == 'geo':
                dict_param['Tile'] = 'tileinfo'
            elif arg == 'pmd':
                dict_param['Tile'] = 'tile_pmd'
            elif arg == 'gdp':
                dict_param['Tile'] = 'tile_gdp'
            elif arg == 'fresco':
                dict_param['Tile'] = 'tile_fresco'
            else:
                print( opt + ' expects geo, pmd, gdp' )
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
        A = year // 100
        B = 2 - A + A // 4
        julian = julian + B
    return julian * 1.0

def JulianGOME( year, month, day, hour, minu ):
    day += (hour + minu / 60.) / 24.
    return JulianAstro( year, month-1, day-1 ) - JulianAstro( 1950, 0, 0 )

#+++++++++++++++++++++++++
def BuildWhereList( dict_param, table_lst ):
    where_lst = []
#
# which (meta-) table are posibly involved
#
    if dict_param['Name']:
        if dict_param['Name'][-1] == '1' \
                or dict_param['Name'][0:16] == 'ER02_GOM_GOM_1P_':
	    dict_param['Level'] = '1'
            metaTable = 'meta__1P.'
        elif dict_param['Name'][-1] == '2' \
                or dict_param['Name'][0:15] == 'GOME_O3-NO2_L2_':
	    dict_param['Level'] = '2'
            metaTable = 'meta__2P.'
        else:
            print( 'Unknown product -> no entry' )
            sys.exit(1)

        mystr = metaTable + 'name=\'' + dict_param['Name'] + '\''
        where_lst.append( mystr )
    elif dict_param['Tile'] == 'tile_fresco':
        metaTable = 'meta_fresco.'
    elif dict_param['Level']:
        if dict_param['Level'] == '1':
            metaTable = 'meta__1P.'
        elif dict_param['Level'] == '2':
            metaTable = 'meta__2P.'
        else:
            print( 'Unknown product -> no entry' )
            sys.exit(1)
    else:
        print( 'Please select at least on name or product level' )
        sys.exit(1)
    tileTable = 'tileinfo.'

# define list of tables
    table_lst.append( metaTable[:-1] )
    table_lst.append( tileTable[:-1] )
#
# selections on metaTable
#
    if dict_param['Received']:
        mystr = metaTable \
                 + 'receiveDate>=\'' + dict_param['Received'] + '\''
        where_lst.append( mystr )

    if dict_param['softVersion']:
        where_lst.append( metaTable + 'softVersion LIKE \'' \
                          + dict_param['softVersion'] + '%\'' )

    if dict_param['Orbit']:
        orbitList = dict_param['Orbit'].split(',')
        if len( orbitList ) == 1:
            mystr = metaTable + 'absOrbit=' + orbitList[0]
        else:
            mystr = metaTable + 'absOrbit BETWEEN '\
                    +  orbitList[0] + ' AND ' + orbitList[1]
        where_lst.append( mystr )
#
# selections on tileinfo
#
    if dict_param['Longitude'] and dict_param['Latitude']:
        mystr = '%stile && ST_GeomFromText(\'POLYGON((' \
                '%g %g,%g %g,%g %g,%g %g,%g %g' \
                '))\',4326)' % ( tileTable, dict_param['Longitude'][1],
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
        mystr = '%stile && ST_GeomFromText(\'POLYGON((' \
                '%g %g,%g %g,%g %g,%g %g,%g %g' \
                '))\',4326)' % ( tileTable, dict_param['Longitude'][1], -90,
                                 dict_param['Longitude'][1], 90,
                                 dict_param['Longitude'][0], 90,
                                 dict_param['Longitude'][0], -90,
                                 dict_param['Longitude'][1], -90 )
        where_lst.append( mystr )
    elif dict_param['Latitude']:
        mystr = '%stile && ST_GeomFromText(\'POLYGON((' \
                '%g %g,%g %g,%g %g,%g %g,%g %g' \
                '))\',4326)' % ( tileTable, 180, dict_param['Latitude'][0],
                                 180, dict_param['Latitude'][1],
                                -180, dict_param['Latitude'][1],
                                -180, dict_param['Latitude'][0],
                                 180, dict_param['Latitude'][0] )
        where_lst.append( mystr )
#
# selections on metaTable or tileinfo
#
    if dict_param['Date']:
        tableName = metaTable
        if dict_param['Output'] != 'tile':
            for mystr in where_lst:
                if mystr.split('.')[0].strip('(') == 'tileinfo':
                    tableName = tileTable
                    break
        else:
            tableName = tileTable

        if len( dict_param['Date'] ) < 4:
            print( 'You should atleast specify a year' )
            sys.exit(1)
        year = int(dict_param['Date'][0:4])
        if year < 1995:
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

        if tableName == metaTable:
            mystr = metaTable + \
                    'dateTimeStart BETWEEN \'%04d-%02d-%02d %02d:%02d:00\'' \
                    % (year, month, day, hour, minu)
        else:
            mystr = tileTable + 'julianDay BETWEEN %g' \
                    % JulianGOME( year, month, day, hour, minu )

        if dtime == 'year':
            year += 1
        elif dtime == 'month':
            month += 1
            if month > 12:
                year += 1
                month = 1
        elif dtime == 'day':
            day += 1
        elif dtime == 'hour':
            hour += 1
            if hour >= 24:
                day += 1
                hour = 0
        else:
            minu += 1
            if minu >= 60:
                hour += 1
                if hour >= 24:
                    day += 1
                    hour = 0
                minu = 0

                minu = 0
        
        if tableName == metaTable:
            mystr += ' AND \'%04d-%02d-%02d %02d:%02d:00\'' % \
                     (year, month, day, hour, minu)
        else:
            mystr += ' AND %g' \
                    % JulianGOME(year, month, day, hour, minu)
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
    tbl_tile   = None
    where_opt  = None
    for table in table_lst:
        if table[0:4] == 'meta':
            tbl_meta = table
            where_opt = ' ORDER BY absOrbit,softVersion'
        elif table[0:4] == 'tile':
            tbl_tile = 'tileinfo'
    #
    # set up SELECT query:
    #
    query_str  = None
    if where_tbl.has_key( tbl_tile ):
        query_str = 'SELECT %s FROM %s WHERE pk_meta IN (%s)' \
                    % (row_lst, tbl_meta, query_str)

    if where_tbl.has_key( tbl_meta ):
        if query_str:
            query_str += ' AND ' + where_tbl[tbl_meta]
        else:
            query_str = 'SELECT %s FROM %s WHERE %s' \
                        % (row_lst, tbl_meta, where_tbl[tbl_meta])
    if not query_str:
        query_str = 'SELECT %s FROM %s' % (row_lst, tbl_meta)

    if where_opt:
        query_str += where_opt

    return query_str

#+++++++++++++++++++++++++
def QueryTileInfo( table_lst, where_tbl ):

    row_lst = 'pk_tileinfo,julianday,release,pixelnumber,subsetcounter,' \
              'swathtype,satzenithangle,sunzenithangle,relazimuthangle,' \
              'asText(tile)'

    # combine where-actions
    tbl_meta = None
    tbl_tile = None
    for table in table_lst:
        if table[0:4] == 'meta':
            tbl_meta = table
        elif table[0:4] == 'tile':
            tbl_tile = 'tileinfo'
    #
    # set up SELECT query:
    #
    query_str  = None
    if where_tbl.has_key( tbl_meta ):
        query_str = 'SELECT pk_meta FROM %s WHERE %s ORDER BY %s' \
                    % (tbl_meta, where_tbl[tbl_meta], 'absOrbit,softVersion')
        query_str = 'SELECT DISTINCT fk_tileinfo FROM tileinfo_%s'\
                    ' WHERE fk_meta in (%s)' % (tbl_meta, query_str)
        query_str = 'SELECT %s FROM tileinfo WHERE pk_tileinfo IN (%s)' \
                    % (row_lst, query_str)

    if where_tbl.has_key( tbl_tile ):
        if query_str:
            query_str += ' AND ' + where_tbl[tbl_tile]
        else:
            query_str = 'SELECT %s FROM tileinfo WHERE %s' \
                        % (row_lst, where_tbl[tbl_tile])
            
    return query_str

#+++++++++++++++++++++++++
def QueryTile( table_lst, where_tbl, tile_tbl ):

    # combine where-actions
    tbl_meta = None
    tbl_tile = None
    for table in table_lst:
        if table[0:4] == 'meta':
            tbl_meta = table
        elif table[0:4] == 'tile':
            tbl_tile = 'tileinfo'

    row_lst = 'pk_tileinfo,ti.julianday,release,pixelnumber,subsetcounter,' \
              'swathtype,satzenithangle,sunzenithangle,relazimuthangle,' \
              'asText(tile)'
    #
    # set up SELECT query:
    #
    query_str = None
    query_meta_str = None
    where_opt = 'absOrbit,softVersion'
    if tile_tbl == 'tile_fresco':
        row_lst += ',integrationTime,errorFlag,cloudFraction,cloudTopHeight'
	row_lst += ',cloudTopPressure,cloudAlbedo'
        row_lst += ',surfaceHeight,surfacePressure,surfaceAlbedo'

        if where_tbl.has_key( tbl_meta ):
            query_meta_str = 'SELECT pk_meta FROM %s WHERE %s ORDER BY %s' \
                             % (tbl_meta, where_tbl[tbl_meta], where_opt)
            query_meta_str = 'SELECT DISTINCT julianDay FROM tile_%s'\
                             ' WHERE fk_meta in (%s)' \
                             % (tbl_meta, query_meta_str)
        if where_tbl.has_key( tbl_tile ):
            if query_meta_str:
                query_str = 'SELECT * FROM tileinfo WHERE' \
                            '(julianDay IN (%s)) AND %s' \
                            % (query_meta_str, where_tbl[tbl_tile])
            else:
                query_str = 'SELECT * FROM tileinfo WHERE %s' \
                            % where_tbl[tbl_tile]
        else:
            query_str = 'SELECT * FROM tileinfo WHERE julianDay IN (%s)' \
                        % query_meta_str
    else:
        if tile_tbl == 'tile_gdp':
            row_lst += ',o3_gdp,o3_gdp_err'\
                       ',cloudfraction,cloudtoppress,surfacepress'
        elif tile_tbl == 'tile_pmd':
            row_lst += ',pmd_1,pmd_2,pmd_3'

        if where_tbl.has_key( tbl_meta ):
            query_meta_str = 'SELECT pk_meta FROM %s WHERE %s ORDER BY %s' \
                             % (tbl_meta, where_tbl[tbl_meta], where_opt)
            query_meta_str = 'SELECT DISTINCT fk_tileinfo FROM tileinfo_%s'\
                             ' WHERE fk_meta in (%s)' \
                             % (tbl_meta, query_meta_str)
        if where_tbl.has_key( tbl_tile ):
            if query_meta_str:
                query_str = 'SELECT * FROM tileinfo WHERE' \
                            '(pk_tileinfo IN (%s)) AND %s' \
                            % (query_meta_str, where_tbl[tbl_tile])
            else:
                query_str = 'SELECT * FROM tileinfo WHERE %s' \
                            % where_tbl[tbl_tile]
        else:
            query_str = 'SELECT * FROM tileinfo WHERE pk_tileinfo IN (%s)' \
                        % query_meta_str

    query_str = 'SELECT %s FROM (%s) ti, %s WHERE pk_tileinfo = fk_tileinfo' \
                % (row_lst,query_str,tile_tbl)

    return query_str

#+++++++++++++++++++++++++
def QueryLast( table_lst, where_tbl ):

    # build a list of rows to be shown by the select statement
    row_lst = 'absOrbit,max(softVersion)'

    # combine where-actions and join table if necessary
    tbl_meta  = None
    where_opt = None
    for table in table_lst:
        if table[0:4] == 'meta':
            tbl_meta = table
            where_opt = ' GROUP BY absOrbit ORDER BY absOrbit'
    
    where_expr = None
    if where_tbl.has_key( tbl_meta ):
        where_expr = ' WHERE ' + where_tbl[tbl_meta] 
    #
    # set up SELECT query as:
    # SELECT 'row_lst' FROM 'from_lst' WHERE 'where_expr' 'where_opt'
    #
    query_str = 'SELECT ' + row_lst + ' FROM ' + tbl_meta
    if where_expr: query_str += where_expr
    if where_opt:  query_str += where_opt

    query2_str = 'SELECT name FROM ' + tbl_meta \
                 + ' WHERE absOrbit=%d AND softVersion=\'%s\''
    return (query_str, query2_str)

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
                      password=db_passwd, database='gome' )
    except psycopg2.InterfaceError:
        print( 'Interface Error: failed to connect as '
               + db_user + ' to ' + db_host + '.gome' )
        print( 'Interface Error: failed to connect' )
        return None
    except psycopg2.DatabaseError:
        print( 'Database Error: failed to connect as '
               + db_user + ' to ' + db_host + '.gome' )
        return None
    except psycopg2.Error:
        print( 'General Error: failed to connect as '
               + db_user + ' to ' + db_host + '.gome' )
        return None
    else:
        if db_user != 'nadc_admin':
            cu = cx.cursor()
            cu.execute( 'SET search_path TO nadc_admin,public' )
            cu.close()
        return cx

#+++++++++++++++++++++++++
def get_nadc_link_path( gomeVersion, gomefl ):

    path = '/nfs/ers/gome/'
    if gomefl[-1] == '1':
        path += 'level1/'
    elif gomefl[-1] == '2':
        path += 'level2/'

    path += gomeVersion.replace('/', '_') + '/'
    if gomefl[0:15] == 'GOME_O3-NO2_L2_':
        year = gomefl[15:19]
        month = gomefl[19:21]
        day = gomefl[21:23]
    elif len(gomefl) == 12:
        tmp_year = int(gomefl[0:1])
        if tmp_year < 5:
            year = '%4d' % ( 2000 + tmp_year)
        else:
            year = '%4d' % ( 1990 + tmp_year)
        month = gomefl[1:3]
        day = gomefl[3:5]
    elif len(gomefl) == 22:
        year = gomefl[0:4]
        month = gomefl[4:6]
        day = gomefl[6:8]
    else:
        year  = '0000'
        month = '00'
        day   = '00'
    path += year + '/'
    path += month + '/'
    path += day + '/'
    return path + gomefl

#+++++++++++++++++++++++++
def get_nadc_path( gomefl ):

    path = '/GOME'
    if gomefl[0:16] == 'ER02_GOM_GOM_1P_' or gomefl[-1] == '1':
        path += '/LV1_01'
    elif gomefl[-1] == '2':
        path += '/LV2_01'

    if gomefl[0:16] == 'ER02_GOM_GOM_1P_':
        year = gomefl[16:20]
        month = gomefl[20:22]
        day = gomefl[22:24]
    elif gomefl[0:15] == 'GOME_O3-NO2_L2_':
        year = gomefl[15:19]
        month = gomefl[19:21]
        day = gomefl[21:23]
    elif len(gomefl) == 12:
        tmp_year = int(gomefl[0:1])
        if tmp_year < 5:
            year = '%4d' % ( 2000 + tmp_year)
        else:
            year = '%4d' % ( 1990 + tmp_year)
        month = gomefl[1:3]
        day = gomefl[3:5]
    elif len(gomefl) == 22:
        year = gomefl[0:4]
        month = gomefl[4:6]
        day = gomefl[6:8]
    else:
        year  = '0000'
        month = '00'
        day   = '00'
    path += '/' + year + '/' + month + '/' + day + '/' 
    return path + gomefl

#+++++++++++++++++++++++++ Main Routine +++++++++++++++++++++++++
def inquire_gome_db( dict_param ):
# ---
# build SELECT-action
    table_lst = []
    where_lst = BuildWhereList( dict_param, table_lst )
    if dict_param['Debug']:
        print( 'where_l(st: ', where_lst, '\n' )

# build where-action split over the main tables (meta)
    where_tbl = {}
    for mystr in where_lst:
        tbl_name = mystr.split('.')[0].strip('(')
        if len(tbl_name.split('(')) == 2: tbl_name = tbl_name.plot('(')[1]
        
        if where_tbl.has_key( tbl_name ):
            mystr += ' AND ' + where_tbl[tbl_name]
        where_tbl[tbl_name] = mystr
    if dict_param['Debug']:
        print( 'where_tbl: ', where_tbl, '\n' )

# build the whole query
    query_str  = None
    query2_str = None
    if dict_param['lastVersion']:
        (query_str,query2_str) = QueryLast( table_lst, where_tbl )
    elif dict_param['Output'] == 'file':
        query_str = QueryFile( table_lst, where_tbl )
    elif dict_param['Output'] == 'header':
        query_str = QueryFile( table_lst, where_tbl, File=False )
    elif dict_param['Output'] == 'tile':
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
    cu = cx.cursor()
    cu.execute( query_str )
    while 1:
        row = cu.fetchone()
        if row == None: break

        if dict_param['lastVersion']:
            cuu = cx.cursor()
            cuu.execute( query2_str % (row[0],row[1]) )
            row2 = cuu.fetchone()
            if row2 == None:
                print( row )
            else:
                print( row2[0] )
            cuu.close()
        elif dict_param['Output'] == 'file':
            gomeVersion = row[1].strip()
            gomefl = row[0].strip()
        
            if dict_param['Link']:
                print( get_nadc_link_path( gomeVersion, gomefl ) )
            else:
                print( get_nadc_path( gomefl ) )
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
    inquire_gome_db( dict_param )
