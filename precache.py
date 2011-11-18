import os
import sys
import hashlib
import json
import re
import argparse
import fnmatch

from utils import get_platform, file_exists, load_config

PRECACHE_FILE_ARCH_BIT = 0
PRECACHE_FILE_PLATFORM_BIT = 4
PRECACHE_FILE_EXECUTE_BIT = 8

# determine platform id
# NOTE: These values MUST match the same values in the precachelib.h
def get_platform_id( pstring ):
	if pstring == "windows":
		return 1
	elif pstring == "linux":
		return 2
	elif pstring == "macosx":
		return 3
	return None
	
# determine architecture id
def get_arch_id( arch ):
	if arch == "x86":
		return 1
	elif arch == "x64":
		return 2
	return None

# condense these ids into a single value
def create_flags( arch_id, os_id, binary ):
	return (int(arch_id) << PRECACHE_FILE_ARCH_BIT | int(os_id) << PRECACHE_FILE_PLATFORM_BIT | int(binary) << PRECACHE_FILE_EXECUTE_BIT)	
	
#
# functions
def make_relative_to( inpath, relpath ):
	if relpath in inpath:
		return inpath[ len(relpath): ]
	else:
		print( 'relpath NOT in inpath' )
		print( '%s <-> %s' % (relpath, inpath) )
	
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


p = argparse.ArgumentParser( description='Generate precache.list for a project' )
#p.add_argument( '-t', '--target', dest='target_path', metavar='TARGET_PATH', required=True )
#p.add_argument( '-o', '--output', dest='output_file', metavar='OUTPUT_FILE', required=True )
p.add_argument( '-f', '--file', dest='config_file', metavar='CONFIG_FILE', required=True )


cmdline = p.parse_args()

ignores = None

	
PRECACHE_FILE = 'precache.list'
output = {}
output['version'] = 1
output['filelist'] = []

cfg = {}
ignore_list = []
actions = None
updaters = None
config_path = os.path.normpath( cmdline.config_file )


def ignores_to_regex( excludes ):
	ignore_list = []
	for e in excludes:
		e = os.path.normpath( e )
		e = e.replace('\\', '\\\\')
		e = e.replace('.', '\\.')
		e = e.replace('*', '.*')
		pat = re.compile( e )
		ignore_list.append( pat )
	return ignore_list

if file_exists( config_path ):
	print( 'Loading configuration from %s...' % config_path )
	cfg = load_config( config_path )
	
		
	cfg['abs_target_path'] = os.path.normpath( os.path.abspath( os.path.dirname( cmdline.config_file ) ) )
	print( 'Absolute target path: %s' % cfg['abs_target_path'] )
	cfg['abs_deploy_path'] = os.path.normpath(os.path.abspath(cfg['abs_target_path'] + '/' + cfg['local_project_path']))
	print( 'Absolute deploy path: %s' % cfg['abs_deploy_path'] )	
	
	excludes = []
	if 'excludes' in cfg:
		excludes = cfg['excludes']
		
	ignore_list = ignores_to_regex( excludes )
		
	actions = []
	if 'actions' in cfg:
		actions = cfg['actions']

	if 'updaters' in cfg:
		if 'updater_path' not in cfg:
			print( 'ERROR: Missing updater_path in config.' )
			sys.exit(1)
			
		updaters = cfg['updaters']
		cfg['abs_updater_path'] = os.path.normpath( cfg['abs_target_path'] + '/' + cfg['updater_path'] )
			

	output['install_path'] = cfg['install_path']
	if output['install_path'][0] is not '/':
		output['install_path'] = '/' + output['install_path']

	output['remote_project_path'] = cfg['remote_project_path']
	if output['remote_project_path'][0] is not '/':
		output['remote_project_path'] = '/' + output['remote_project_path']
else:
	print( 'Configuration %s not found' % config_path )

cfg['output_file'] = os.path.normpath( cfg['abs_target_path'] + '/' + PRECACHE_FILE )
	

def add_file_data( fullpath, relative_path, filedata ):
	filedata['path'] = relative_path
	filedata['md5'] = md5_from_file(fullpath)
	return filedata
	
def add_file( fullpath, relative_path, filedata ):
	filedata = add_file_data( fullpath, relative_path, filedata )
	output['filelist'].append( filedata )
	
def add_updater( fullpath, relative_path, filedata ):
	filedata = add_file_data( fullpath, relative_path, filedata )
	output['updaters'].append( filedata )
	
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
if actions != None:
	for platform_string in actions:
		platform_actions = actions[ platform_string ]
		for action in platform_actions:
			
			filedata = {}
			filedata['target'] = action['file']
			
			if 'target' in action:
				filedata['target'] = action['target']		
			
			run_bit = 0
			if 'run' in action:
				run_bit = action['run']
				
			arch_bit = 0
			if 'arch' in action:
				arch_bit = get_arch_id( action['arch'] )
			
			filedata['flags'] = create_flags( arch_bit, get_platform_id(platform_string), run_bit )
			relative_path = action['file'].replace("\\", "/")
			fullpath = (cfg['abs_deploy_path'] + action['file']).replace("\\", "/")
			
			print( 'fullpath: %s' % fullpath )
			add_file( fullpath, relative_path, filedata )
			
if updaters != None and 'updater_path' in cfg:
	output['updaters'] = []
	print( 'Process updaters...' )
	print( cfg['updater_path'] )
	for platform_string in updaters:
		update_entry = updaters[ platform_string ]
		ignore_list = []
		run_list = []
		
		if 'ignores' in update_entry:
			ignore_list = ignores_to_regex( update_entry['ignores'] )
			
		if 'runlist' in update_entry:
			run_list = ignores_to_regex( update_entry['runlist'] )
	
		entry_path = update_entry[ 'src' ]
		elpath = os.path.normpath( cfg['abs_updater_path'] + entry_path )
		print( elpath )

		for root, dirs, files in os.walk( elpath ):
			for file in files:
				path_ignored = False
				in_runlist = False
				fullpath = root + os.path.sep + file
				
				# check to see if we should ignore this
				for i in ignore_list:
					if i.match( fullpath ):
						path_ignored = True
						break
						
				for i in run_list:
					if i.match( fullpath ):
						in_runlist = True
						break			
						
				if not path_ignored:
					print( fullpath )
					relative_path = make_relative_to( fullpath, os.path.normpath(cfg['abs_updater_path'] + entry_path) )
					relative_path = relative_path.replace( "\\", "/" )
					print( relative_path )
					
					filedata = {}
					arch_bit = 0
					run_bit = 0
					if in_runlist:
						run_bit = 1
						print( 'This file should be EXECUTABLE' )
						
					
					filedata['target'] = relative_path
					filedata['flags'] = create_flags( arch_bit, get_platform_id(platform_string), run_bit )					
					
					add_updater( fullpath, relative_path, filedata )

jsondata = json.dumps(output, indent=4, sort_keys=True)

print( 'Writing precache file: %s' % cfg['output_file'] )
out = open( cfg['output_file'], 'wb' )
out.write( jsondata )
out.close()
