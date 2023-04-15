# watchdog
[![Build Status](https://github.com/Keidan/watchdog/actions/workflows/linux.yml/badge.svg)][linuxCI]
[![Release](https://img.shields.io/github/v/release/Keidan/watchdog.svg?logo=github)][releases]
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)][license]
[![Bugs](https://sonarcloud.io/api/project_badges/measure?project=Keidan_watchdog&metric=bugs)][sonarcloud]
[![Code Smells](https://sonarcloud.io/api/project_badges/measure?project=Keidan_watchdog&metric=code_smells)][sonarcloud]
[![Duplicated Lines (%)](https://sonarcloud.io/api/project_badges/measure?project=Keidan_watchdog&metric=duplicated_lines_density)][sonarcloud]
[![Vulnerabilities](https://sonarcloud.io/api/project_badges/measure?project=Keidan_watchdog&metric=vulnerabilities)][sonarcloud]
[![Maintainability Rating](https://sonarcloud.io/api/project_badges/measure?project=Keidan_watchdog&metric=sqale_rating)][sonarcloud]
[![Reliability Rating](https://sonarcloud.io/api/project_badges/measure?project=Keidan_watchdog&metric=reliability_rating)][sonarcloud]
[![Security Rating](https://sonarcloud.io/api/project_badges/measure?project=Keidan_watchdog&metric=security_rating)][sonarcloud]
[![Technical Debt](https://sonarcloud.io/api/project_badges/measure?project=Keidan_watchdog&metric=sqale_index)][sonarcloud]
[![Lines of Code](https://sonarcloud.io/api/project_badges/measure?project=Keidan_watchdog&metric=ncloc)][sonarcloud]
[![Coverage](https://sonarcloud.io/api/project_badges/measure?project=Keidan_watchdog&metric=coverage)][sonarcloud]
[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=Keidan_watchdog&metric=alert_status)][sonarcloud]

(GPL) Simple process watchdog.

This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.


## Instructions

download the software :

	mkdir devel
	cd devel
	
	git clone git://github.com/Keidan/watchdog.git
	cd watchdog
	cmake -S . -B build -DDISTRIBUTION=[debug|release] -DCONFIG_DIR=[/etc] -G"Ninja"
	cmake --build .
	Or use with vscode
	
	Supported options:
	* Supported conf dir.: -DCONFIG_DIR=[/etc/watchdog]
	* Supported distrib.: -DDISTRIBUTION=[debug|release]
	* Supported arch.: -DCMAKE_BUILD_TYPE=[x86|x86_64|armv7l]
	* Disable XML: -DDISABLE_XML=on
	* Disable JSON: -DDISABLE_JSON=on


## Usage

_watchdog options_:
- --help, -h: Print this help.
- --version, -v: Print the version.
- --disable-spam-detect: Disable spam detection.
- --pid: Write the pid in the file pointed by pidfile.
- --pidfile: The file containing the pid (default: /var/run/watchdog.pid) (option enables pid support)
- --max-respawn: The maximum number of respawn allowed (default: 5; 0 for a start without respawn)
- --min-respawn-delay: The minimum respawn delay before starting spam detection (default: 500 miliseconds)

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

If no configuration file is passed as a parameter, the application looks for a configuration file located in the [CONFIG_DIR] directory and named: ```<the_application_name>.[xml/json]```.<br/>

It's possible to create symbolic links with several configuration files:<br/>
```ln -s watchdog watchdog-foo```.<br/>
```watchdog-foo --new```.<br/>
You can now edit the configuration file ```watchdog-foo.[xml/json]```.<br/>
 	

## License

[GPLv3](https://github.com/Keidan/watchdog/blob/master/LICENSE)

[linuxCI]: https://github.com/Keidan/watchdog/actions?query=workflow%3ALinux
[sonarcloud]: https://sonarcloud.io/summary/new_code?id=Keidan_watchdog
[releases]: https://github.com/Keidan/watchdog/releases
[license]: https://github.com/Keidan/watchdog/blob/master/license.txt