import os
import platform
import json

# -----------------------------------------------------------------------------
# Utils
# -----------------------------------------------------------------------------

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
	
def load_config( file ):
	f = open( file, "rb" )
	data = f.read()
	f.close()
	d = json.loads( data )
	return d