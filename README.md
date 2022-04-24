# log_over_http NONOS_SDK app (example)

log output over http for esp8266.

## Usage

* `cd /you/path/ESP8266_NONOS_SDK`
* `git clone https://github.com/slacky1965/log_over_http.git`
* `cd log_over_http`

* Go to `project/user/include/wifi.h`
	1. edit to wifi.h for you settings.
	
* `make all` for compiling applications
* `make flash` to download the app
* In order to test the file server demo
	. test the example interactively on a web browser (assuming IP is 192.168.100.40):
		- open path `http://192.168.100.40` or `http://192.168.100.40/index.html` to see an HTML web page

* `make clean` - default clean

	
