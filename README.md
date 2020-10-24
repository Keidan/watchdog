# watchdog
[![Build Status](https://travis-ci.com/Keidan/watchdog.svg?branch=master)](https://travis-ci.com/Keidan/watchdog)

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
	
	Supported options:
	* Supported conf dir.: -DCONFIG_DIR=[/etc/watchdog]
	* Supported distrib.: -DDISTRIBUTION=[debug|release]
	* Supported arch.: -DCMAKE_BUILD_TYPE=[x86|x86_64|armv7l]
	* Disable XML: -DDISABLE_XML=yes
	* Disable JSON: -DDISABLE_JSON=yes


## Usage

_watchdog options_:
- --help, -h: Print this help.
- --disable-spam-detect: Disable spam detection.
- --pid: Write the pid in the file pointed by pidfile.
- --pidfile: The file containing the pid (default: /var/run/watchdog.pid) (option enables pid support)
- --max-respawn: The maximum number of respawn allowed (default: 5; 0 for a start without respawn)
- --min-respawn-delay: The minimum respawn delay before starting spam detection (default: 0.05 nanosec)

Stand-alone mode:
- --path, -p: The process path (optional if the binary is in the PATH).
- --working, -w: The working directory (optional).
- --name, -n: The name of the process.
- --arg, -a: The process argument (repeat if there're several).
- --env, -e: The process environment variable (repeat if there're several).

File mode:
- --directory, -d: The directory to find the configuration file (default: [CONFIG_DIR]).
- --file-type, -t: The file type (possible values: xml or json).
- --config, -c: Loads a configuration file.
- --new, -z: Creates a new configuration file.

If no configuration file is passed as parameter..<br/>
The application looks for a configuration file located in the [CONFIG_DIR] directory and named: ```<the_application_name>.[xml/json]```.<br/>
It's possible to create symbolic links with several configuration files:.<br/>
```ln -s watchdog watchdog-foo```.<br/>
```watchdog-foo --new```.<br/>
you now can edit the configuration file /home/keidan/watchdog/watchdog-foo.[xml/json].<br/>
 	

## License

[GPLv3](https://github.com/Keidan/watchdog/blob/master/LICENSE)
