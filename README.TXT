
Building
   First build x264 and ffmpeg in the usual cmake way, then do a top level cmake build.

TODO
   - We need a "commander" process that runs the show. The "commander" should:
      - Discover cameras (and periodically rediscover them)
      - Implement API's
         - GET discovered cameras
         - GET assigned cameras
         - POST assign camera
         - DELETE camera
      - Periodically hit vss api's and make sure correct cameras are assigned.
   - We need analytics
      - Get h.264 keyframes from vss
         - Decode
            - Detect Motion
               - Database Motion
   - We need a viewing web app
      - We possibly need more advanced "player" javascript.
      - Features
         - View discovered cameras
         - Assign camera
         - View assigned cameras
         - View live thumbnails of all cameras (with periodic updates)
         - View live video
            - Timeline showing holes
   - Raspberry Pi
      - Use hardware encoder
      - Use hardware decoder
   
Some random API'S
   http://127.0.0.1:11002/data_sources
   http://127.0.0.1:11003/jpg_before?data_source_id=c0f367c4-b360-4524-91df-d5e1fa52cd14&time=2018-11-10T13:34:00.000&width=320&height=240
   http://127.0.0.1:11003/video?data_source_id=c0f367c4-b360-4524-91df-d5e1fa52cd14&start_time=2018-11-12T08:46:00.000&end_time=2018-11-12T08:47:00.000&file_path=/home/td/export.mp4






M-SEARCH * HTTP/1.1
ST: upnp:rootdevice
MX: 5
MAN: "ssdp:discover"
HOST: 239.255.255.250:1900



HTTP/1.1 200 OK
ST: upnp:rootdevice
LOCATION: http://192.168.0.1:5000/Public_UPNP_gatedesc.xml
SERVER: Linux/2.6.12, UPnP/1.0, NETGEAR-UPNP/1.0
EXT:
CACHE-CONTROL: max-age=3600
USN: uuid:26909ec8-b06b-8c63-3e5c-d98b8eecf32b::upnp:rootdevice



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: upnp:rootdevice
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a::upnp:rootdevice



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: upnp:rootdevice
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a::upnp:rootdevice



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: uuid:db379abc-3d72-54a5-b731-100d7f5b570a
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: uuid:db379abc-3d72-54a5-b731-100d7f5b570a
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: urn:schemas-wifialliance-org:device:WFADevice:1
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a::urn:schemas-wifialliance-org:device:WFADevice:1



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: urn:schemas-wifialliance-org:device:WFADevice:1
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a::urn:schemas-wifialliance-org:device:WFADevice:1



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: urn:schemas-wifialliance-org:service:WFAWLANConfig:1
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a::urn:schemas-wifialliance-org:service:WFAWLANConfig:1



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: urn:schemas-wifialliance-org:service:WFAWLANConfig:1
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a::urn:schemas-wifialliance-org:service:WFAWLANConfig:1



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1801
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://192.168.1.109:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: upnp:rootdevice
USN: uuid:29813fcf-e851-9a32-fd8d-ae642fd93579::upnp:rootdevice



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=120
ST: upnp:rootdevice
USN: uuid:00000::upnp:rootdevice
EXT:
SERVER: AirTies/ASP 1.0 UPnP/1.0 MiniUPnPd/1.6
LOCATION: http://192.168.1.109:5555/rootDesc.xml
OPT: "http://schemas.upnp.org/upnp/1/0/";
01-NLS: 1
BOOTID.UPNP.ORG: 1
CONFIGID.UPNP.ORG: 1337



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
LOCATION: http://192.168.1.78:80/Device.xml
SERVER: argus-discserver
ST: upnp:rootdevice
USN: 0800C007-8000-0001-0802-788602E20002::upnp:argusdevice



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
LOCATION: http://192.168.1.78:80/Device.xml
SERVER: argus-discserver
ST: upnp:rootdevice
USN: 0800C007-8000-0001-0802-788602E20002::upnp:argusdevice



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1801
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://192.168.1.109:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: upnp:rootdevice
USN: uuid:29813fcf-e851-9a32-fd8d-ae642fd93579::upnp:rootdevice



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1801
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://192.168.1.109:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: uuid:29813fcf-e851-9a32-fd8d-ae642fd93579
USN: uuid:29813fcf-e851-9a32-fd8d-ae642fd93579



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1801
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://192.168.1.109:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: uuid:29813fcf-e851-9a32-fd8d-ae642fd93579
USN: uuid:29813fcf-e851-9a32-fd8d-ae642fd93579



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
LOCATION: http://192.168.1.78:80/Device.xml
SERVER: argus-discserver
ST: upnp:rootdevice
USN: 0800C007-8000-0001-0802-788602E20002::upnp:argusdevice



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
LOCATION: http://192.168.1.78:80/Device.xml
SERVER: argus-discserver
ST: upnp:rootdevice
USN: 0800C007-8000-0001-0802-788602E20002::upnp:argusdevice



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1801
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://192.168.1.109:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: urn:schemas-wifialliance-org:device:WFADevice:1
USN: uuid:29813fcf-e851-9a32-fd8d-ae642fd93579::urn:schemas-wifialliance-org:device:WFADevice:1



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1801
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://192.168.1.109:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: urn:schemas-wifialliance-org:device:WFADevice:1
USN: uuid:29813fcf-e851-9a32-fd8d-ae642fd93579::urn:schemas-wifialliance-org:device:WFADevice:1



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1801
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://192.168.1.109:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: urn:schemas-wifialliance-org:service:WFAWLANConfig:1
USN: uuid:29813fcf-e851-9a32-fd8d-ae642fd93579::urn:schemas-wifialliance-org:service:WFAWLANConfig:1



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1801
DATE: Mon, 10 Dec 2018 03:13:16 GMT
EXT:
LOCATION: http://192.168.1.109:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: urn:schemas-wifialliance-org:service:WFAWLANConfig:1
USN: uuid:29813fcf-e851-9a32-fd8d-ae642fd93579::urn:schemas-wifialliance-org:service:WFAWLANConfig:1



HTTP/1.1 200 OK
HOST: 239.255.255.250:1900
EXT:
CACHE-CONTROL: max-age=100
LOCATION: http://192.168.1.106:80/description.xml
SERVER: Linux/3.14.0 UPnP/1.0 IpBridge/1.24.0
hue-bridgeid: 001788FFFE7D899C
ST: upnp:rootdevice
USN: uuid:2f402f80-da50-11e1-9b23-0017887d899c::upnp:rootdevice



HTTP/1.1 200 OK
HOST: 239.255.255.250:1900
EXT:
CACHE-CONTROL: max-age=100
LOCATION: http://192.168.1.106:80/description.xml
SERVER: Linux/3.14.0 UPnP/1.0 IpBridge/1.24.0
hue-bridgeid: 001788FFFE7D899C
ST: uuid:2f402f80-da50-11e1-9b23-0017887d899c
USN: uuid:2f402f80-da50-11e1-9b23-0017887d899c



HTTP/1.1 200 OK
HOST: 239.255.255.250:1900
EXT:
CACHE-CONTROL: max-age=100
LOCATION: http://192.168.1.106:80/description.xml
SERVER: Linux/3.14.0 UPnP/1.0 IpBridge/1.24.0
hue-bridgeid: 001788FFFE7D899C
ST: urn:schemas-upnp-org:device:basic:1
USN: uuid:2f402f80-da50-11e1-9b23-0017887d899c



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:18 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: upnp:rootdevice
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a::upnp:rootdevice



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:18 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: upnp:rootdevice
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a::upnp:rootdevice



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:18 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: uuid:db379abc-3d72-54a5-b731-100d7f5b570a
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:18 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: uuid:db379abc-3d72-54a5-b731-100d7f5b570a
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:18 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: urn:schemas-wifialliance-org:device:WFADevice:1
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a::urn:schemas-wifialliance-org:device:WFADevice:1



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:18 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: urn:schemas-wifialliance-org:device:WFADevice:1
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a::urn:schemas-wifialliance-org:device:WFADevice:1



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:18 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: urn:schemas-wifialliance-org:service:WFAWLANConfig:1
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a::urn:schemas-wifialliance-org:service:WFAWLANConfig:1



HTTP/1.1 200 OK
CACHE-CONTROL: max-age=1800
DATE: Mon, 10 Dec 2018 03:13:18 GMT
EXT:
LOCATION: http://169.254.36.203:49152/wps_device.xml
SERVER: Unspecified, UPnP/1.0, Unspecified
ST: urn:schemas-wifialliance-org:service:WFAWLANConfig:1
USN: uuid:db379abc-3d72-54a5-b731-100d7f5b570a::urn:schemas-wifialliance-org:service:WFAWLANConfig:1



HTTP/1.1 200 OK
HOST: 239.255.255.250:1900
EXT:
CACHE-CONTROL: max-age=100
LOCATION: http://192.168.1.106:80/description.xml
SERVER: Linux/3.14.0 UPnP/1.0 IpBridge/1.24.0
hue-bridgeid: 001788FFFE7D899C
ST: upnp:rootdevice
USN: uuid:2f402f80-da50-11e1-9b23-0017887d899c::upnp:rootdevice



HTTP/1.1 200 OK
HOST: 239.255.255.250:1900
EXT:
CACHE-CONTROL: max-age=100
LOCATION: http://192.168.1.106:80/description.xml
SERVER: Linux/3.14.0 UPnP/1.0 IpBridge/1.24.0
hue-bridgeid: 001788FFFE7D899C
ST: uuid:2f402f80-da50-11e1-9b23-0017887d899c
USN: uuid:2f402f80-da50-11e1-9b23-0017887d899c



HTTP/1.1 200 OK
HOST: 239.255.255.250:1900
EXT:
CACHE-CONTROL: max-age=100
LOCATION: http://192.168.1.106:80/description.xml
SERVER: Linux/3.14.0 UPnP/1.0 IpBridge/1.24.0
hue-bridgeid: 001788FFFE7D899C
ST: urn:schemas-upnp-org:device:basic:1
USN: uuid:2f402f80-da50-11e1-9b23-0017887d899c



HTTP/1.1 200 OK
HOST: 239.255.255.250:1900
EXT:
CACHE-CONTROL: max-age=100
LOCATION: http://192.168.1.106:80/description.xml
SERVER: Linux/3.14.0 UPnP/1.0 IpBridge/1.24.0
hue-bridgeid: 001788FFFE7D899C
ST: upnp:rootdevice
USN: uuid:2f402f80-da50-11e1-9b23-0017887d899c::upnp:rootdevice



HTTP/1.1 200 OK
HOST: 239.255.255.250:1900
EXT:
CACHE-CONTROL: max-age=100
LOCATION: http://192.168.1.106:80/description.xml
SERVER: Linux/3.14.0 UPnP/1.0 IpBridge/1.24.0
hue-bridgeid: 001788FFFE7D899C
ST: uuid:2f402f80-da50-11e1-9b23-0017887d899c
USN: uuid:2f402f80-da50-11e1-9b23-0017887d899c



HTTP/1.1 200 OK
HOST: 239.255.255.250:1900
EXT:
CACHE-CONTROL: max-age=100
LOCATION: http://192.168.1.106:80/description.xml
SERVER: Linux/3.14.0 UPnP/1.0 IpBridge/1.24.0
hue-bridgeid: 001788FFFE7D899C
ST: urn:schemas-upnp-org:device:basic:1
USN: uuid:2f402f80-da50-11e1-9b23-0017887d899c



HTTP/1.1 200 OK
HOST: 239.255.255.250:1900
EXT:
CACHE-CONTROL: max-age=100
LOCATION: http://192.168.1.106:80/description.xml
SERVER: Linux/3.14.0 UPnP/1.0 IpBridge/1.24.0
hue-bridgeid: 001788FFFE7D899C
ST: upnp:rootdevice
USN: uuid:2f402f80-da50-11e1-9b23-0017887d899c::upnp:rootdevice



HTTP/1.1 200 OK
HOST: 239.255.255.250:1900
EXT:
CACHE-CONTROL: max-age=100
LOCATION: http://192.168.1.106:80/description.xml
SERVER: Linux/3.14.0 UPnP/1.0 IpBridge/1.24.0
hue-bridgeid: 001788FFFE7D899C
ST: uuid:2f402f80-da50-11e1-9b23-0017887d899c
USN: uuid:2f402f80-da50-11e1-9b23-0017887d899c



HTTP/1.1 200 OK
HOST: 239.255.255.250:1900
EXT:
CACHE-CONTROL: max-age=100
LOCATION: http://192.168.1.106:80/description.xml
SERVER: Linux/3.14.0 UPnP/1.0 IpBridge/1.24.0
hue-bridgeid: 001788FFFE7D899C
ST: urn:schemas-upnp-org:device:basic:1
USN: uuid:2f402f80-da50-11e1-9b23-0017887d899c

