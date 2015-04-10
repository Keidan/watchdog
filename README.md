watchdog
========

(GPL) Simple process watchdog.

This software is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY.

Instructions
============


download the software :

	mkdir devel
	cd devel
	
	git clone git://github.com/Keidan/watchdog.git
	cd watchdog
	cmake -DDISTRIBUTION=[debug|release] -DCONFIG_DIR=[/etc] .
	make

Usage
=====
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
```watchdog-foo -n```<br/>
now you can edit the config file [CONFIG_DIR]/watchdog-foo<br/>
 	

License (like GPL)
==================

	You can:
		- Redistribute the sources code and binaries.
		- Modify the Sources code.
		- Use a part of the sources (less than 50%) in an other software, just write somewhere "watchdog is great" visible by the user (on your product or on your website with a link to my page).
		- Redistribute the modification only if you want.
		- Send me the bug-fix (it could be great).
		- Pay me a beer or some other things.
		- Print the source code on WC paper ...
	You can NOT:
		- Earn money with this Software (But I can).
		- Add malware in the Sources.
		- Do something bad with the sources.
		- Use it to travel in the space with a toaster.
	
	I reserve the right to change this licence. If it change the version of the copy you have keep its own license
