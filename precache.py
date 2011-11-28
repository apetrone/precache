import os
import sys

import json

import argparse
import fnmatch
import posixpath

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
	
	cfg['abs_deploy_base'] = os.path.normpath(os.path.abspath(cfg['abs_target_path'] + '/' + cfg['local_project_path']))
	cfg['abs_content_path'] = os.path.normpath( cfg['abs_deploy_base'] + '/' + cfg['content_path'] )

	print( 'Absolute deploy path: %s' % cfg['abs_deploy_base'] )	
	
	
	
	
	# convert excludes
	ignore_list = ignores_to_regex( cfg['excludes'] )

	if 'content_path' not in cfg:
		print( 'ERROR: content_path missing from config' )
		sys.exit(1)
	
	if 'binary_source' not in cfg:
		cfg['binary_source'] = cfg['content_path']
	
	if 'binary_target' not in cfg:
		cfg['binary_target'] = cfg['binary_source']
		
	if cfg['binary_target'][0] != '/':
		cfg['binary_target'] = '/' + cfg['binary_target']
		
	if 'updater_path' in cfg:
		cfg['abs_updater_path'] = os.path.normpath( cfg['abs_target_path'] + '/' + cfg['updater_path'] )
			

			
	output['install_path'] = cfg['install_path']
	if output['install_path'][0] is not '/':
		output['install_path'] = '/' + output['install_path']

	output['remote_project_path'] = cfg['remote_project_path']
	if output['remote_project_path'][0] is not '/':
		output['remote_project_path'] = '/' + output['remote_project_path']
		
		
	if 'platforms' not in cfg:
		cfg['platforms'] = [ get_platform() ]
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


def traverse_files( source, ignore_list, arch_agnostic=True, current_platform=None, source_prefix=None, target_prefix=None ):
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
					else:
						arch_bit = 0
						
				platform_id = 0
				if current_platform != None:
					platform_id = get_platform_id( current_platform )
				
				flags = create_flags( arch_bit, platform_id )
				if flags > 0:
					filedata['flags'] = flags
				
				relative_path = make_relative_to( fullpath, source )
				relative_path = relative_path.replace("\\", "/")

				if source_prefix != None:									
					relative_path = '/' + source_prefix + relative_path
					relative_path = relative_path.replace( '//', '/' )
					#print( 'relative_path=%s' % relative_path )

					
				if target_prefix != None:
					filedata['target'] = target_prefix + relative_path
				else:
					filedata['target'] = relative_path

				if filedata['target'] == relative_path:
					del filedata['target']

						
				add_file( fullpath, relative_path, filedata )


def traverse_binaries( source, binary_source, platforms, ignore_list=[] ):
	abs_bin_path = os.path.normpath( source + '/' + binary_source )
	
	target_prefix = cfg['binary_target']
	
	for platform in platforms:
				
		'''
		vars['PLATFORM'] = platform
				
		# the source folder to start from
		abs_binary_source = os.path.normpath( var_replace(abs_bin_path, vars) + '/' )
		'''		
				
		# the source folder to start from
		abs_binary_source = os.path.normpath( abs_bin_path )
		
		# prefix to relative file path
		source_prefix = binary_source + '/'
		
		#print( 'source_prefix=%s' % source_prefix )
		print( 'abs_binary_source=%s' % abs_binary_source )
		traverse_files( abs_binary_source, ignore_list, arch_agnostic=False, current_platform=platform )

			
# content from main deploy path
content_ignore_list = ignore_list[:]
binary_path_match = '*/%s/*' % cfg['binary_source']
content_ignore_list.extend( ignores_to_regex( [ binary_path_match ] ) )
print( 'abs_content_path=%s' % cfg['abs_content_path'] )
traverse_files( cfg['abs_content_path'], content_ignore_list )

print( 'abs_deploy_base=%s' % cfg['abs_deploy_base'] )
# binaries for each platform
traverse_binaries( cfg['abs_deploy_base'], cfg['binary_source'], cfg['platforms'], ignore_list )

			
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
