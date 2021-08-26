from os import getenv, path

qt_version = "5.15.2"
qtc_version = "5.0.0"
os_name = getenv('OS', 'linux')
qt_dir = path.abspath('qt')
qt_modules = ['qtbase', 'qttools', 'icu']
plugin_name = 'QtcUtilities'
pro_file = path.abspath(path.dirname(__file__) + '/../../qtc-utilities.pro')
ts_files_dir = path.abspath(path.dirname(__file__) + '/../../translation')
