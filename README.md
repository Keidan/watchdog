# watchdog
[![Build Status](https://img.shields.io/travis/Keidan/hex2bin/master.svg?style=plastic)](https://travis-ci.org/Keidan/watchdog)
[![GitHub license](https://img.shields.io/github/license/Keidan/hex2bin.svg?style=plastic)](https://github.com/Keidan/watchdog/blob/master/LICENSE)

(GPL) Simple process watchdog.

This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.

## Instructions


download the software :

	mkdir devel
	cd devel
	
	git clone git://github.com/Keidan/watchdog.git
	cd watchdog
	cmake -DDISTRIBUTION=[debug|release] -DCONFIG_DIR=[/etc] .
	make

## Usage

_watchdog options_:
- --help, -h: Print this help.

Mode standalone:
- --path, -p: The process path (optional if the binary is in the PATH).
- --name, -n: The process name.
- --arg, -a: The process argument (repeat for more).
- --env, -e: The process environment variable (repeat for more).

Mode file:
- --config, -c: Load a config file.
- --new, -n: Create a new config file.

If no configuration file is passed as parameter.<br/>
The application search a configuration file localized into the folder [CONFIG_DIR] and named: ```<the_application_name>.xml```<br/>
It's possible to create symbolic links with several configuration files:<br/>
```ln -s watchdog watchdog-foo```<br/>
```watchdog-foo --new```<br/>
now you can edit the config file [CONFIG_DIR]/watchdog-foo.xml<br/>
 	

## License

[GPLv3](https://github.com/Keidan/watchdog/blob/master/LICENSE)
