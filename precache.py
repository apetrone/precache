import os
import sys
import hashlib
import json
import re
import platform
import argparse

def get_platform():
	p = platform.platform().lower()
	#print( "Platform: " + p )
	if 'linux' in p:
		return "linux"
	elif "darwin" in p:
		return "macosx"	
	elif "nt" or "windows" in p:
		return "windows"
	else:
		return "unknown"

		


def loadConfig( file ):
	f = open( file, "rb" )
	data = f.read()
	f.close()
	d = json.loads( data )
	return d

#
# functions
def make_relative_to( inpath, relpath ):
	if relpath in inpath:
		return inpath[ len(relpath): ]

	
def md5_from_file( file ):
	hash = ''
	block_size = 8192
	m = hashlib.md5()
	
	f = open( file, "rb" )
	
	while True:
		data = f.read( block_size )
		if not data:
			break
		m.update(data)
	
	f.close()
	
	
	return m.hexdigest()
				

CONFIG_FILE = 'precache.conf'
PLATFORM_ID = 0


p = argparse.ArgumentParser( description='Generate precache.list for a project' )
#p.add_argument( '-t', '--target', dest='target_path', metavar='TARGET_PATH', required=True )
#p.add_argument( '-o', '--output', dest='output_file', metavar='OUTPUT_FILE', required=True )
p.add_argument( '-f', '--file', dest='config_file', metavar='CONFIG_FILE', required=True )


cmdline = p.parse_args()

ignores = None

	
PRECACHE_FILE = 'precache.list'
output = {}
output['version'] = 1
output['base'] = ''
output['filelist'] = []
cfg = {}
ignore_list = []
map = None
config_path = os.path.normpath( cmdline.config_file )


if os.path.exists(config_path) and os.stat( config_path ):
	print( 'Loading configuration from %s...' % config_path )
	cfg = loadConfig( config_path )
	
	excludes = []
	if 'excludes' in cfg:
		excludes = cfg['excludes']
	
	for e in excludes:
		
		e = os.path.normpath( e )
		e = e.replace('\\', '\\\\')
		e = e.replace('.', '\\.')
		e = e.replace('*', '.*')
		pat = re.compile( e )
		ignore_list.append( pat )
		
	map = []
	if 'map' in cfg:
		map = cfg['map']
		myos = get_platform()
		if myos in map:
			map = map[ myos ]
		
		
	cfg['abs_target_path'] = os.path.normpath( os.path.abspath( os.path.dirname( cmdline.config_file ) ) )
	print( 'Absolute target path: %s' % cfg['abs_target_path'] )
	cfg['abs_deploy_path'] = os.path.normpath(os.path.abspath(cfg['abs_target_path'] + '/' + cfg['target_path']))
	print( 'Absolute deploy path: %s' % cfg['abs_deploy_path'] )

	output['base'] = cfg['basename']
else:
	print( 'Configuration %s not found' % config_path )

cfg['output_file'] = os.path.normpath( cfg['abs_target_path'] + '/' + PRECACHE_FILE )



# determine platform id
# NOTE: These values MUST match the same values in the precache.h
pstring = get_platform()
if pstring == "windows":
	PLATFORM_ID = 0
elif pstring == "linux":
	PLATFORM_ID = 1
elif pstring == "macosx":
	PLATFORM_ID = 2

def add_file( fullpath, relative_path, filedata ):
	filedata['path'] = relative_path
	filedata['md5'] = md5_from_file(fullpath)
	output['filelist'].append( filedata )
for root, dirs, files in os.walk( cfg['abs_deploy_path'] ):
	for f in files:
		path_ignored = False
		fullpath = root + os.path.sep + f
		
		for i in ignore_list:
			if i.match( fullpath ):
				#print( 'MATCH: %s' % i.pattern )
				#print( 'ignored: %s' % fullpath )
				path_ignored=True
				break
		
		if not path_ignored:
			relative_path = make_relative_to( fullpath, cfg['abs_deploy_path'] )
			relative_path = relative_path.replace("\\", "/")
			add_file( fullpath, relative_path, {} )

# take care of platform-specific files
if map != None:
	for src in map:
		dst = map[src]
		fullpath = os.path.abspath(cfg['abs_deploy_path'] + '/' + src)
		relative_path = dst
		relative_path = relative_path.replace("\\", "/")
		filedata = {}
		filedata['platform'] = str(PLATFORM_ID)
		add_file( fullpath, relative_path, filedata )

jsondata = json.dumps(output, indent=4, sort_keys=True)

print( 'Writing precache file: %s' % cfg['output_file'] )
out = open( cfg['output_file'], 'wb' )
out.write( jsondata )
out.close()
