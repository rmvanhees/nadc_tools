#!/usr/bin/env python2
#
# (c) SRON - Netherlands Institute for Space Research (2016).
# All Rights Reserved.
# This software is distributed under the BSD 2-clause license.
#
#* Function   : inquire_scia_db
#* Instrument : Sciamachy NRT/Offline products of level 0, 1b and 2
#* Purpose    : perform a number of predefined database queries
#* Usage      : inquire_scia_db [-h] [--debug] {name,type} ...
#*
#*   Subcommand "name" : selection on product name:
#*     optional arguments:
#*       -h, --help            show this help message and exit
#*       --stateID STATEID     selection on stateID (comma-separated)
#*       --lat LAT             selection on latitude [-90,90]
#*       --lon LON             selection on longitude <-180,180]
#*       -o {meta,state}, --output {meta,state}
#*                             select information to be returned, default: state
#*
#*   Subcommand "type" : selection on product type {0,1,2}
#*     optional arguments:
#*       -h, --help            show this help message and exit
#*       --best                lists names of highest consolidated products, 
#*                             accepts orbit and date selection, other options 
#*                             are neglected
#*       --orbit ORBIT         selection on orbit number or range
#*       --procStage PROCSTAGE
#*                             selection on ProcStage (comma-separated)
#*       --softVersion SOFTVERSION
#*                             selection on S/W version (as SCIA/x.xx)
#*       --stateID STATEID     selection on stateID (comma-separated)
#*       --obsmode {nadir,limb,occultation,monitor}
#*                             selection on observation mode
#*       --date DATE           select entries on start time of science data;
#*            [yyyy]: selection between yyyy and (yyyy+1)
#*            [yyyymm]: selection between yyyymm and yyyy(mm+1)
#*            [yyyymmdd]: selection between yyyymmdd and yyyymm(dd+1)
#*            [yyyymmddhh]: selection between yyyymmddhh and yyyymmdd(hh+1)
#*            [yyyymmddhhmm]: selection between yyyymmddhh and yyyymmddhh(mm+1)
#*       --rtime {1h,2h,3h,4h,5h,6h,7h,8h,9h,10h,11h,12h,13h,14h,15h,16h,17h,
#*                18h,19h,20h,21h,22h,23h,1d,2d,3d,4d,5d,6d,7d}
#*                             select entries on receive time: xh or xd
#*       --lat LAT             selection on latitude [-90,90]
#*       --lon LON             selection on longitude <-180,180]
#*       -o {product,meta,state}, --output {product,meta,state}
#*                             select information to be returned, default: product
#
#
from __future__ import print_function
from __future__ import division

import sys
import psycopg2

from datetime import datetime, timedelta

#+++++++++++++++++++++++++
def get_product_by_name( args ):
    if args.product[0:10] == 'SCI_NL__0P':
        metaTBL = 'meta__0P'
    elif args.product[0:10] == 'SCI_NL__1P':
        metaTBL = 'meta__1P'
    else:
        metaTBL = 'meta__2P'
    meta_where = "{}.name=\'{}\'".format( metaTBL, args.product )
    stateTBL = 'stateinfo'

    # Selection (only) on stateTable
    where_state = []
    if args.stateID is not None:
        if len( args.stateID.split(',') ) == 1:
            mystr = '{}.stateID={}'.format(stateTBL, args.stateID)
        else:
            mystr = '{}.stateID IN ({})'.format(stateTBL, args.stateID)
        where_state.append( mystr )

    if args.lon != [-180., 180.] or args.lat != [-90., 90.]:
        mystr = "{}.tile && ST_GeomFromText(\'POLYGON((" \
                "{:g} {:g},{:g} {:g},{:g} {:g},{:g} {:g},{:g} {:g}))\'" \
                ",4326)"
        mystr = mystr.format( stateTBL, args.lon[1], args.lat[0],
                              args.lon[1], args.lat[1],
                              args.lon[0], args.lat[1],
                              args.lon[0], args.lat[0],
                              args.lon[1], args.lat[0] )
        where_state.append( mystr )
 
    # Combine where-statements for stateTable
    if where_state == []:
        state_where = ''
    else:
        state_where = ''
        for mystr in where_state:
            if len(state_where) > 1:
                state_where += ' AND '
            state_where += mystr
            
        if args.output == 'state':
            state_where = 'AND ' + state_where
        else:
            state_where = 'WHERE ' + state_where

    # Show debug info
    if args.debug:
        print( 'meta_where: ', meta_where )
        print( 'where_state: ', where_state )
        print( 'state_where: "{}"'.format(state_where) )

    # Compose query string
    if args.output == 'state':
        row_lst = 'stateID,absOrbit,dateTimeStart,muSecStart,dateTimeStop,' \
                  'muSecStop,timeLine,orbitPhase,softVersion,obmTemp,' \
                  'detTemp,pmdTemp,ST_asText(tile)'
        query_str = 'SELECT {} FROM stateinfo WHERE pk_stateinfo IN' \
                    + ' (SELECT unnest(fk_stateinfo) FROM {} LEFT JOIN' \
                    + ' stateinfo_{} ON {}.pk_meta=stateinfo_{}.fk_meta' \
                    + ' WHERE {}) {} ORDER BY dateTimeStart'
        query_str = query_str.format( row_lst, 
                                      metaTBL, metaTBL, metaTBL, metaTBL, 
                                      meta_where, state_where )
    else:
        if args.output == 'meta':
            row_lst = 'name,fileSize,procStage,softVersion,absOrbit,' \
                      'receiveDate,dateTimeStart,dateTimeStop,numDataSets,' \
                      'nadirStates,limbStates,OccultStates,monitorStates'
        else:             # product
            row_lst = 'name'

        if state_where == '':
            query_str = 'SELECT {} FROM {} WHERE {}'.format( row_lst, metaTBL,
                                                             meta_where )
        else:
            query_str = 'WITH sm AS (SELECT fk_meta,procStage,unnest(fk_stateinfo)' \
                        + ' AS fk_stateinfo FROM stateinfo_{} WHERE fk_meta' \
                        + ' IN (SELECT pk_meta FROM {} WHERE {})), ss' \
                        + ' AS (SELECT pk_stateinfo,softVersion FROM stateinfo {})' \
                        + ' SELECT {} FROM {} WHERE pk_meta IN' \
                        + ' (SELECT DISTINCT fk_meta FROM sm JOIN ss ON' \
                        + ' sm.fk_stateinfo=pk_stateinfo)'
            query_str = query_str.format( metaTBL, metaTBL, meta_where,
                                          state_where, row_lst, metaTBL )
    return query_str
    
#+++++++++++++++++++++++++
def get_product_by_type( args ):
    where_meta = []
    where_state = []
    if args.type == 0:
        metaTBL = 'meta__0P'
    elif args.type == 1:
        metaTBL = 'meta__1P'
    else:
        metaTBL = 'meta__2P'
    stateTBL = 'stateinfo'

    # Selections (only) on metaTable
    if args.rtime is not None:
        mystr = '{}.receiveDate>=\'{}\''.format(metaTBL, args.rtime)
        where_meta.append( mystr )

    if args.softVersion is not None:
        mystr = '{}.softVersion like \'{}%\''.format(metaTBL, args.softVersion)
        where_meta.append( mystr )

    if args.stateID is not None:
        if len( args.stateID.split(',') ) == 1:
            mystr = '{}.stateID={}'.format(stateTBL, args.stateID)
        else:
            mystr = '{}.stateID IN ({})'.format(stateTBL, args.stateID)
        where_state.append( mystr )

    if args.obsmode is not None:
        if args.obsmode == 'nadir':
            state_str = '1,2,3,4,5,6,7,11,12,13,14,15'
        elif args.obsmode == 'limb':
            state_str = '28,29,30,31,32,33'
        elif args.obsmode == 'occultation':
            state_str = '56,57'
            mystr = '{}.stateID IN (56,57)'
        elif args.obsmode == 'monitor':
            state_str = '8,16,26,39,46,48,51,52,59,61,62,63,65,69,70'
        where_state.append( '{}.stateID IN ({})'.format(stateTBL, state_str) )

    if args.lon != [-180., 180.] or args.lat != [-90., 90.]:
        mystr = "{}.tile && ST_GeomFromText(\'POLYGON((" \
                "{:g} {:g},{:g} {:g},{:g} {:g},{:g} {:g},{:g} {:g}))\'" \
                ",4326)"
        mystr = mystr.format( stateTBL, args.lon[1], args.lat[0],
                              args.lon[1], args.lat[1],
                              args.lon[0], args.lat[1],
                              args.lon[0], args.lat[0],
                              args.lon[1], args.lat[0] )
        where_state.append( mystr )

    # Selection on metaTable or stateTable
    if args.procStage is not None:
        res = args.procStage.split(',')
        if len( res ) == 1:
            mystr = "substr({}.softVersion,{},1)=\'{}\'".format(stateTBL,
                                                                args.type+1,
                                                                res[0])
        else:
            mystr = 'substr({}.softVersion,{},1) in ('.format(stateTBL,
                                                              args.type+1)
            for c in res:
                if mystr[-1] != '(':  mystr += ','
                mystr += "\'" + c + "\'"
            mystr += ')'
        where_state.append( mystr )

        if len( res ) == 1:
            mystr = "{}.procStage=\'{}\'".format(metaTBL, res[0])
        else:
            mystr = '{}.procStage IN ('.format(metaTBL)
            for c in res:
                if mystr[-1] != '(':  mystr += ','
                mystr += "\'" + c + "\'"
            mystr += ')'
        where_meta.append( mystr )

    if args.orbit is not None:
        if len(args.orbit) == 1:
            mystr = '{}.absOrbit={}'.format(stateTBL, args.orbit[0])
        else:
            mystr = '{}.absOrbit BETWEEN {} and {}'.format(stateTBL,
                                                           args.orbit[0],
                                                           args.orbit[1])
        where_state.append( mystr )

        if len(args.orbit) == 1:
            mystr = '{}.absOrbit={}'.format(metaTBL, args.orbit[0])
        else:
            mystr = '{}.absOrbit BETWEEN {} and {}'.format(metaTBL,
                                                           args.orbit[0],
                                                           args.orbit[1])
        where_meta.append( mystr )
    
    if args.date is not None:
        if len(args.date) == 12:
            d1 = datetime.strptime( args.date, '%Y%m%d%H%M' )
            d2 = d1 + timedelta(minutes=1)
        elif len(args.date) == 10:
            d1 = datetime.strptime( args.date, '%Y%m%d%H' )
            d2 = d1 + timedelta(hours=1)
        elif len(args.date) == 8:
            d1 = datetime.strptime( args.date, '%Y%m%d' )
            d2 = d1 + timedelta(days=1)
        elif len(args.date) == 6:
            d1 = datetime.strptime( args.date, '%Y%m' )
            if d1.month == 11:
                d2 = datetime( d1.year+1, 1, 1, 0, 0, 0 )
            else:
                d2 = datetime( d1.year, d1.month+1, 1, 0, 0, 0 )
        elif len(args.date) == 4:
            d1 = datetime.strptime( args.date, '%Y' )
            d2 = datetime( d1.year+1, 1, 1, 0, 0, 0 )
        else:
            print( '*** Fatal: invalid date specification' )
            sys.exit(1)

        mystr = '{}.dateTimeStart BETWEEN \'{}\' AND \'{}\''.format(stateTBL,
                                                            d1.isoformat(' '),
                                                            d2.isoformat(' ') )
        where_state.append( mystr )

        mystr = '{}.dateTimeStart BETWEEN \'{}\' AND \'{}\''.format(metaTBL,
                                                            d1.isoformat(' '),
                                                            d2.isoformat(' ') )
        where_meta.append( mystr )

    # Combine where-statements for metaTable
    if where_meta == []:
        meta_where = ''
    else:
        meta_where = "WHERE "
        for mystr in where_meta:
            if len( meta_where ) > 6:
                meta_where += ' AND '
            meta_where += mystr

    # Combine where-statements for stateTable
    # * reject rows for which we do not have any data (column "softVersion")
    # * do not perform previous check when a selection of "softVersion" is requested
    # * perform no selection on "softVersion" when it is the only selection-key
    if where_state == []:
        state_where = ''
    else:
        found_proc = False
        for mystr in where_state:
            if mystr.find('softVersion') > 0:
                found_proc = True
                
        if len(where_state) == 1 and found_proc:
            state_where = ''
        else:
            state_where = "WHERE "
            for mystr in where_state:
                if len( state_where ) > 6:
                    state_where += ' AND '
                state_where += mystr

            if not found_proc:
                state_where += " AND substr({}.softVersion,{},1) != \'0\'".format(stateTBL,
                                                                                  args.type+1)
    # Show debug info
    if args.debug:
        print( 'where_meta: ', where_meta )
        print( 'meta_where : "{}"'.format(meta_where) )
        print( 'where_state: ', where_state )
        print( 'state_where: "{}"'.format(state_where) )

    # Compose query string
    if args.output == 'state':
        row_lst = 'stateID,absOrbit,dateTimeStart,muSecStart,dateTimeStop,' \
                  'muSecStop,timeLine,orbitPhase,softVersion,obmTemp,' \
                  'detTemp,pmdTemp,ST_asText(tile)'
        query_str = 'SELECT {} FROM stateinfo {} ORDER BY dateTimeStart'
        query_str = query_str.format( row_lst, state_where )
    else:
        if args.output == 'meta':
            row_lst = 'name,fileSize,procStage,softVersion,absOrbit,' \
                      'receiveDate,dateTimeStart,dateTimeStop,numDataSets,' \
                      'nadirStates,limbStates,OccultStates,monitorStates'
        else:             # product
            row_lst = 'name'

        if state_where == '':
            query_str = 'SELECT {} FROM {} {}'.format( row_lst, metaTBL,
                                                       meta_where )
        else:
            query_str = 'WITH sm AS (SELECT fk_meta,procStage,unnest(fk_stateinfo)' \
                        + ' AS fk_stateinfo FROM stateinfo_{} WHERE fk_meta' \
                        + ' IN (SELECT pk_meta FROM {} {})), ss' \
                        + ' AS (SELECT pk_stateinfo,softVersion FROM stateinfo {})' \
                        + ' SELECT {} FROM {} WHERE pk_meta IN' \
                        + ' (SELECT DISTINCT fk_meta FROM sm JOIN ss ON' \
                        + ' sm.fk_stateinfo=pk_stateinfo)'
            query_str = query_str.format( metaTBL, metaTBL, meta_where,
                                          state_where, row_lst, metaTBL )

        query_str += ' ORDER BY absOrbit,procStage,softVersion'

    return query_str

#+++++++++++++++++++++++++
def get_best_products( args ):
    where_meta = []
    if args.type == 0:
        metaTBL = 'meta__0P'
    elif args.type == 1:
        metaTBL = 'meta__1P'
    else:
        metaTBL = 'meta__2P'
    stateTBL = 'stateinfo'

    # selection (only) on stateTable
    where_state = ["substr({}.softVersion,{},1) <> \'0\'".format(stateTBL, args.type+1)]
    
    # selection on metaTable or stateTable
    if args.orbit is not None:
        if len(args.orbit) == 1:
            mystr = '{}.absOrbit={}'.format(stateTBL, args.orbit[0])
        else:
            mystr = '{}.absOrbit BETWEEN {} and {}'.format(stateTBL,
                                                           args.orbit[0],
                                                           args.orbit[1])
        where_state.append( mystr )

        if len(args.orbit) == 1:
            mystr = '{}.absOrbit={}'.format(metaTBL, args.orbit[0])
        else:
            mystr = '{}.absOrbit BETWEEN {} and {}'.format(metaTBL,
                                                           args.orbit[0],
                                                           args.orbit[1])
        where_meta.append( mystr )
    
    if args.date is not None:
        if len(args.date) == 12:
            d1 = datetime.strptime( args.date, '%Y%m%d%H%M' )
            d2 = d1 + timedelta(minutes=1)
        elif len(args.date) == 10:
            d1 = datetime.strptime( args.date, '%Y%m%d%H' )
            d2 = d1 + timedelta(hours=1)
        elif len(args.date) == 8:
            d1 = datetime.strptime( args.date, '%Y%m%d' )
            d2 = d1 + timedelta(days=1)
        elif len(args.date) == 6:
            d1 = datetime.strptime( args.date, '%Y%m' )
            if d1.month == 11:
                d2 = datetime( d1.year+1, 1, 1, 0, 0, 0 )
            else:
                d2 = datetime( d1.year, d1.month+1, 1, 0, 0, 0 )
        elif len(args.date) == 4:
            d1 = datetime.strptime( args.date, '%Y' )
            d2 = datetime( d1.year+1, 1, 1, 0, 0, 0 )
        else:
            print( '*** Fatal: invalid date specification' )
            sys.exit(1)

        mystr = '{}.dateTimeStart BETWEEN \'{}\' AND \'{}\''.format(stateTBL,
                                                            d1.isoformat(' '),
                                                            d2.isoformat(' ') )
        where_state.append( mystr )

        mystr = '{}.dateTimeStart BETWEEN \'{}\' AND \'{}\''.format(metaTBL,
                                                            d1.isoformat(' '),
                                                            d2.isoformat(' ') )
        where_meta.append( mystr )

    # combine where-statements for Meta and State table
    if where_meta == []:
        meta_where = ''
    else:
        meta_where = 'WHERE '
        for mystr in where_meta:
            if len( meta_where ) > 6:
                meta_where += ' AND '
            meta_where += mystr

    if where_state == []:
        state_where = ''
    else:
        state_where = 'WHERE '
        for mystr in where_state:
            if len( state_where ) > 6:
                state_where += ' AND '
            state_where += mystr

    # compose query string
    query_str = 'WITH sm AS (SELECT fk_meta,procStage,unnest(fk_stateinfo)' \
                + ' AS fk_stateinfo FROM stateinfo_{} WHERE fk_meta' \
                + ' IN (SELECT pk_meta FROM {} {})), ss' \
                + ' AS (SELECT pk_stateinfo,softVersion FROM stateinfo {})' \
                + ' SELECT name FROM {} WHERE pk_meta IN' \
                + ' (SELECT DISTINCT fk_meta FROM sm JOIN ss ON' \
                + ' sm.fk_stateinfo=pk_stateinfo WHERE' \
                + ' sm.procStage::text=substr(softVersion,{},1))'
    query_str = query_str.format( metaTBL, metaTBL, meta_where, state_where,
                                  metaTBL, args.type+1 )
    query_str += ' ORDER BY absOrbit'

    return query_str

#+++++++++++++++++++++++++
def connect_nadc_db( db_host=None, db_user=None, db_passwd=None ):
    from os.path import isfile
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

#+++++++++++++++++++++++++ Main Routine +++++++++++++++++++++++++
def inquire_scia_db( args ):

    # obtain query string
    if 'product' in args:
        query_str = get_product_by_name( args )
    else:
        if args.best:
            query_str = get_best_products( args )
        else:
            query_str = get_product_by_type( args )
    if args.debug:
        print( '\nquery_str: ', query_str )
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
        if row == None:
            break

        if args.output == 'product':
            print( row[0].strip() )
        else:
            print( ' '.join(str(x) for x in row) )

    cu.close()
    cx.close()

#+++++++++++++++++++++++++
def scia_orbit_range(string):
    res = [int(str) for str in string.split(',')]
    if len(res) > 2:
        msg = '%r is not a orbit number or range' % string
        raise argparse.ArgumentTypeError(msg)
    if len(res) == 2 and res[0] > res[1]: res.sort()
    return res

def scia_lon_range(string):
    res = [float(str) for str in string[1:-1].split(',')]
    if len(res) > 2:
        msg = '%r is not a orbit number or range' % string
        raise argparse.ArgumentTypeError(msg)
    elif len(res) == 1:
        res = [ res[0] - 0.1, res[0] + 0.1 ]
    else:
        if len(res) == 2 and res[0] > res[1]: res.sort()
        
    return res

def scia_lat_range(string):
    res = [float(str) for str in string.split(',')]
    if len(res) > 2:
        msg = '%r is not a orbit number or range' % string
        raise argparse.ArgumentTypeError(msg)
    elif len(res) == 1:
        res = [ res[0] - 0.1, res[0] + 0.1 ]
    else:
        if len(res) == 2 and res[0] > res[1]: res.sort()
        
    return res

#+++++++++++++++++++++++++
if __name__ == '__main__':
    import argparse
    
    parser = argparse.ArgumentParser()
    parser.add_argument( '--debug', action='store_true', default=False,
                         help='show SQL query, but do nothing' )

    # define subparsers for queries on product name
    subparsers = parser.add_subparsers( title='subcommands',
                                        help='select on product name or type' )
    parser_name = subparsers.add_parser( 'name',
                                    help='perform selection on product name' )
    parser_name.add_argument( 'product', type=str,
                              help='name of product to select (no path!)' )
    parser_name.add_argument( '--stateID', type=str, 
                              help='selection on stateID (comma-separated)' )
    parser_name.add_argument( '--lat', default=[-90., 90.],
                              type=scia_lat_range,
                              help='selection on latitude [-90,90]' )
    parser_name.add_argument( '--lon', default=[-180., 180.],
                              type=scia_lon_range,
                              help='selection on longitude <-180,180]' )
    parser_name.add_argument( '-o', type=str, default='state',
                              choices=['meta', 'state'],
                              help='select information to be returned' )
    
    # define subparsers for queries on product type
    parser_type = subparsers.add_parser( 'type',
                                help='perform selection on product level',
                                formatter_class=argparse.RawTextHelpFormatter)
    parser_type.add_argument( 'type', nargs='?', type=int, 
                              choices=[0, 1, 2],
                              help='level of product to select' )
    parser_type.add_argument( '--best', action='store_true', default=False,
                        help='lists names of highest consolidated products' \
                              + ', accepts orbit and date selection' \
                              + ', other options are neglected' )
    parser_type.add_argument( '--orbit', type=scia_orbit_range,
                              help='selection on orbit number or range' )
    parser_type.add_argument( '--procStage', type=str,
                              help='selection on ProcStage (comma-separated)' )
    parser_type.add_argument( '--softVersion', type=str,
                              help='selection on S/W version (as SCIA/x.xx)' )
    parser_type.add_argument( '--stateID', type=str, 
                              help='selection on stateID (comma-separated)' )
    parser_type.add_argument( '--obsmode', type=str,
                              choices=['nadir','limb','occultation','monitor'],
                              help='selection on observation mode' )
    parser_type.add_argument( '--date', type=str,
                         help='''select entries on start time of science data:
        [yyyy]: selection between yyyy and (yyyy+1);
        [yyyymm]: selection between yyyymm and yyyy(mm+1);
        [yyyymmdd]: selection between yyyymmdd and yyyymm(dd+1);
        [yyyymmddhh]: selection between yyyymmddhh and yyyymmdd(hh+1);
        [yyyymmddhhmm]: selection between yyyymmddhhmm and yyyymmddhh(mm+1)
                         ''')
    rtime_opts = ('1h','2h','3h','4h','5h','6h','7h','8h','9h','10h',
                  '11h','12h','13h','14h','15h','16h','17h','18h','19h','20h',
                  '21h','22h','23h','1d','2d','3d','4d','5d','6d','7d')
    parser_type.add_argument( '--rtime', type=str, choices=rtime_opts,
                              help='select entries on receive time: xh or xd' )
    parser_type.add_argument( '--lat', default=[-90., 90.],
                              type=scia_lat_range,
                              help='selection on latitude [-90,90]' )
    parser_type.add_argument( '--lon', default=[-180., 180.],
                              type=scia_lon_range,
                              help='selection on longitude <-180,180]' )
    parser_type.add_argument( '-o', type=str, default='product',
                              choices=['product', 'meta', 'state'],
                              help='select information to be returned' )
    args = parser.parse_args()
    if args.debug:
        print( args )

    inquire_scia_db( args )
