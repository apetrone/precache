#!/usr/bin/env python
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
	#print( 'Absolute target path: %s' % cfg['abs_target_path'] )

	# convert excludes
	ignore_list = ignores_to_regex( cfg['excludes'] )

	if 'updater_path' in cfg:
		cfg['abs_updater_path'] = os.path.normpath( cfg['abs_target_path'] + '/' + cfg['updater_path'] )

	if 'binary_path' not in cfg:
		cfg['binary_path'] = ''

	output['install_path'] = cfg['install_path']
	if output['install_path'][0] is not '/':
		output['install_path'] = '/' + output['install_path']

	output['remote_project_path'] = cfg['remote_project_path']
	if output['remote_project_path'][0] is not '/':
		output['remote_project_path'] = '/' + output['remote_project_path']


	if 'platforms' not in cfg:
		cfg['platforms'] = [ get_platform() ]

	cfg['abs_deploy_base'] = os.path.normpath(os.path.abspath(cfg['abs_target_path'] + '/' + cfg['local_project_path']))
	cfg['abs_content_path'] = os.path.normpath( cfg['abs_deploy_base'] + '/' )

	#print( 'Absolute deploy path: %s' % cfg['abs_deploy_base'] )

else:
	print( 'Configuration %s not found' % config_path )
	sys.exit(1)

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
	#print( 'Traversing: %s' % source )

	for root, dirs, files in os.walk( source ):
		for f in files:
			path_ignored = False
			fullpath = root + os.path.sep + f

			#print( 'fullpath=%s' % fullpath )
			for i in ignore_list:
				if i.match( fullpath ):
					#print( 'Ignoring path %s -> %s' % (fullpath, i) )
					path_ignored=True
					break

			if not path_ignored:
				filedata = {}

				# determine file mode permissions
				owner,group,other = get_mode_for_file( fullpath )
				chmod = ('%s%s%s' % (owner, group, other))
				if chmod != default_file_mode():
					filedata['mode'] = chmod

				execute_bit = 0
				if (owner & 1) > 0 or (group & 1) > 0:
					execute_bit = 1

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

				flags = create_flags( arch_bit, platform_id, execute_bit )
				if flags > 0:
					filedata['flags'] = flags

				relative_path = make_relative_to( fullpath, source )
				relative_path = relative_path.replace("\\", "/")


				if target_prefix != None:
					filedata['target'] = target_prefix + relative_path
				else:
					filedata['target'] = relative_path

				if filedata['target'][0] != '/':
					filedata['target'] = '/' + filedata['target']


				if source_prefix != None:
					relative_path = '/' + source_prefix + relative_path
					relative_path = relative_path.replace( '//', '/' )


				if filedata['target'] == relative_path:
					del filedata['target']


				add_file( fullpath, relative_path, filedata )


def traverse_binaries( source, binaries, ignore_list=[] ):
	for platform in binaries.keys():

		# the source folder to start from
		abs_binary_source = os.path.normpath( source + '/' + binaries[ platform ] )

		# prefix to relative file path
		source_prefix = binaries[ platform ]
		target_prefix = cfg['binary_path']

		traverse_files( abs_binary_source, ignore_list, arch_agnostic=False, current_platform=platform, source_prefix=source_prefix, target_prefix=target_prefix )

# content from main deploy path
content_ignore_list = ignore_list[:]

# ignore any folder specified in the binaries folder, we traverse these separately
if 'binaries' in cfg:
	for entry in cfg['binaries'].keys():
		binary_path_match = '*/%s/*' % cfg['binaries'][ entry ]
		content_ignore_list.extend( ignores_to_regex( [ binary_path_match ] ) )

# traverse main content files
traverse_files( cfg['abs_content_path'], content_ignore_list )

# traverse binaries for each platform; if specified in the config
if 'binaries' in cfg:
	traverse_binaries( cfg['abs_deploy_base'], cfg['binaries'], ignore_list )

jsondata = json.dumps(output, indent=4, sort_keys=True)

print( 'Writing precache file: %s' % cfg['output_file'] )
out = open( cfg['output_file'], 'wb' )
out.write( jsondata )
out.close()
