from os import getenv, path

qt_version = "5.14.2"
qtc_version = "4.12.0"
os_name = getenv('OS', 'linux')
qt_modules = ['qtbase', 'qttools', 'icu']
plugin_name = 'QtcUtilities'
pro_file = path.abspath(path.dirname(__file__) + '/../../qtc-utilities.pro')
ts_files_dir = path.abspath(path.dirname(__file__) + '/../../translation')
