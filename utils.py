import os
import platform
import json
import sys
import re
import hashlib	

# NOTE: These values MUST match the same values in the precachelib.h
PRECACHE_FILE_ARCH_BIT = 0
PRECACHE_FILE_PLATFORM_BIT = 4
PRECACHE_FILE_EXECUTE_BIT = 8

# -----------------------------------------------------------------------------
# Utils
# -----------------------------------------------------------------------------

def require_config( cfg, key ):
	""" Alert the user when a config parameter is missing """
	if key not in cfg:
		print( 'ERROR: Missing config parameter %s' % (key) )
		sys.exit(1)	

def get_platform():
	p = platform.platform().lower()

	if 'linux' in p:
		return "linux"
	elif "darwin" in p:
		return "macosx"	
	elif "nt" or "windows" in p:
		return "windows"
	else:
		return "unknown"
		
def file_exists( path ):
	return os.path.exists(path) and os.stat(path)

def md5_from_file( file ):
	""" Calculate an md5 hash of a file """
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
	
def load_config( file ):
	"""
		Load precache configuration
	"""
	f = open( file, "rb" )
	data = f.read()
	f.close()
	d = json.loads( data )
	
	require_config( d, 'local_project_path' )
	require_config( d, 'install_path' )
	require_config( d, 'remote_project_path' )
	
	# extend the exclude list
	exclude_list = []
	if 'excludes' in d:
		exclude_list.extend( d['excludes'] )
		
	if 'precache_binary_path' in d:
		exclude_list.append( ('*/%s/*' % d['precache_binary_path']) )
		
	if 'precache_launcher_path' in d:
		exclude_list.append( ('*/%s/*' % d['precache_launcher_path']) )	
		
	d['excludes'] = exclude_list
	return d
	

def ignores_to_regex( excludes ):
	""" Convert list of ignores to regex objects """
	ignore_list = []
	for e in excludes:
		e = os.path.normpath( e )
		e = e.replace('\\', '\\\\')
		e = e.replace('.', '\\.')
		e = e.replace('*', '.*')
		pat = re.compile( e )
		ignore_list.append( pat )
	return ignore_list

def get_platform_id( pstring ):
	""" 
		determine platform id
		NOTE: These values MUST match the same values in the precachelib.h
	"""
	if pstring == "windows":
		return 1
	elif pstring == "linux":
		return 2
	elif pstring == "macosx":
		return 3
	return None
	

def get_arch_id( arch ):
	""" 
		determine architecture id
		NOTE: These values MUST match the same values in the precachelib.h
	"""
	if arch == "x86":
		return 1
	elif arch == "x64":
		return 2
	return None



def create_flags( arch_id, os_id, binary ):
	""" condense these ids into a single value """
	return (int(arch_id) << PRECACHE_FILE_ARCH_BIT | int(os_id) << PRECACHE_FILE_PLATFORM_BIT | int(binary) << PRECACHE_FILE_EXECUTE_BIT)	


def make_relative_to( inpath, relpath ):
	""" return a relative path to the inpath, given a relative to source project path """
	if relpath in inpath:
		return inpath[ len(relpath): ]
	else:
		print( 'relpath NOT in inpath' )
		print( '%s <-> %s' % (relpath, inpath) )