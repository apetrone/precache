import os
import sys
import hashlib
import json

argc = len(sys.argv)
if argc < 2:
	print( "usage: python precache.py /path/to/target <target_name>" )
	sys.exit(1)
	
path = sys.argv[1]
base_folder = None

# if base_folder is specified
if argc == 3:
	base_folder = sys.argv[2]
else:
	# if the path has a trailing slash, trim it off
	if path[-1] == os.path.sep:
		path = path[:-1]
	
	last_slash = path.rfind(os.path.sep)
	if last_slash is not -1:
		base_folder = "/" + path[ (last_slash+1): ]
	
# debug
#print( "Path: " + path );
#print( "Base Folder: " + base_folder )

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
		
	
		
output = {}
output['version'] = 1
output['base'] = base_folder
output['filelist'] = []


for root, dirs, files in os.walk( path ):
	for f in files:
		fullpath = root + os.path.sep + f
		relative_path = make_relative_to( fullpath, path )
		relative_path = relative_path.replace("\\", "/")
		
		filedata = {}
		filedata['path'] = relative_path
		filedata['md5'] = md5_from_file(fullpath)
		output['filelist'].append( filedata )

jsondata = json.dumps(output, indent=4, sort_keys=True)


print(jsondata)
