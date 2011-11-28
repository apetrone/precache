import os
import sys

import json

import argparse
import fnmatch

from utils import get_platform, get_platform_id, get_arch_id, get_mode_for_file, file_exists, load_config, ignores_to_regex, md5_from_file, create_flags, make_relative_to, default_file_mode


ignores = None

	
PRECACHE_FILE = 'precache.list'
output = {}
output['version'] = 1
output['filelist'] = []

cfg = {}
ignore_list = []

updaters = None

if __name__ == "__main__":
	p = argparse.ArgumentParser( description='Generate precache.list for a project' )
	p.add_argument( '-f', '--file', dest='config_file', metavar='CONFIG_FILE', required=True )
	cmdline = p.parse_args()

config_path = os.path.normpath( cmdline.config_file )




if file_exists( config_path ):
	print( 'Loading configuration from %s...' % config_path )
	cfg = load_config( config_path )
	
		
	cfg['abs_target_path'] = os.path.normpath( os.path.abspath( os.path.dirname( cmdline.config_file ) ) )
	print( 'Absolute target path: %s' % cfg['abs_target_path'] )
	cfg['abs_deploy_path'] = os.path.normpath(os.path.abspath(cfg['abs_target_path'] + '/' + cfg['local_project_path']))
	print( 'Absolute deploy path: %s' % cfg['abs_deploy_path'] )	
	
	
	
	
	# convert excludes
	ignore_list = ignores_to_regex( cfg['excludes'] )

	if 'binary_path' not in cfg:
		print( 'ERROR: Missing binary_path in config.' )
		sys.exit(1)
		
		
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


def traverse_files( source, ignore_list, arch_agnostic=True, current_platform=None, source_prefix=None ):
	print( 'Traversing: %s' % source )
	
	for root, dirs, files in os.walk( source ):
		for f in files:
			path_ignored = False
			fullpath = root + os.path.sep + f
			
			for i in ignore_list:
				if i.match( fullpath ):
					path_ignored=True
					break
			
			if not path_ignored:
				filedata = {}
				chmod = get_mode_for_file( fullpath )
				#print( fullpath )
				#print( 'Permissions %s' % (chmod) )
				if chmod != default_file_mode():
					filedata['mode'] = chmod
					
				
					
				arch_bit = 0
				if not arch_agnostic:
					if 'x86' in fullpath:
						arch_bit = get_arch_id( 'x86' )
					elif 'x64' in fullpath:
						arch_bit = get_arch_id( 'x64' )
						
				platform_id = 0
				if current_platform != None:
					platform_id = get_platform_id( current_platform )
				
				flags = create_flags( arch_bit, platform_id )
				if flags > 0:
					filedata['flags'] = flags
				
				relative_path = make_relative_to( fullpath, source)
				relative_path = relative_path.replace("\\", "/")
				
				if source_prefix != None:
					filedata['target'] = relative_path
					relative_path = '/' + source_prefix + relative_path

				add_file( fullpath, relative_path, filedata )


def traverse_binaries( source, binary_path, platforms ):
	abs_bin_path = os.path.normpath( source + '/' + binary_path )
	for platform in platforms:
		# the source folder to start from
		binary_source = os.path.normpath( abs_bin_path + '/' + platform )
		
		# prefix to relative file path
		source_prefix = binary_path + '/' + platform
		traverse_files( binary_source, [], arch_agnostic=False, current_platform=platform, source_prefix=source_prefix )

			
# content from main deploy path
traverse_files( cfg['abs_deploy_path'], ignore_list )

# binaries for each platform
traverse_binaries( cfg['abs_deploy_path'], cfg['binary_path'], cfg['platforms'] )

			
'''

			
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
					
					chmod = get_mode_for_file( fullpath )
					print( 'Permissions %s' % (chmod) )
					if chmod != default_file_mode():
						filedata['mode'] = chmod
					filedata['target'] = relative_path
					filedata['flags'] = create_flags( arch_bit, get_platform_id(platform_string) )
					
					add_updater( fullpath, relative_path, filedata )
'''
jsondata = json.dumps(output, indent=4, sort_keys=True)

print( 'Writing precache file: %s' % cfg['output_file'] )
out = open( cfg['output_file'], 'wb' )
out.write( jsondata )
out.close()
