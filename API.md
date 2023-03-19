
# API
This page describes NixieClock REST API
## WiFi scan
Get information on available WiFi networks
**URI**: `/rest/scan`
**Method**: `GET`
### Success response
**Code**: `200`
#### Content example:
When two networks are detected the outout may look like:
```json
[
	{
		"ssid": "networkName1",
		"rssi": -55,
		"open": true
	},
	{
		"ssid": "networkName2",
		"rssi": -70,
		"open": false
	}
]
```
When scan did not yet finish, or nothing was found the response will return empty array `[]`

## WiFi credentials storage
**URI**: `/rest/wifi_cred`
**Method**: `POST`
### `POST` data example
WiFi credentials of the selected network:
```json
{
	"ssid": "********",
	"password": "********"
}
```
### Success response
**Code**: `200`
Body:
```json
{
	"status": true,
	"message": "Connecting to network..."
}
```
### Error response
**Code**: `400`
Body:
```json
{
	"status": false,
	"message": "eror description"
}
```

## WiFi status
**URI**: `/rest/wifi_status`
**Method**: `GET`
### Success response
**Code**: `200`
#### Content example:
```json
{
	"client": {
		"status": 3,
		"ssid": "Livebox-8EA0",
		"rssi": -53,
		"ip": "192.168.1.16"
	},
	"ap": {
		"status": 0,
		"ssid": "",
		"clients": 0,
		"lifetime": -1
	}
}
```
where:
* `"client.status"`:

|Value|              Meaning                  |
|:---:|---------------------------------------|
|  0  | temporary status during configuration |
|  1  | when no SSID is available             |
|  2  | network scan scan is complete         |
|  3  | **connected to a WiFi network**       |
|  4  | the connection fails for all attempts |
|  5  | the connection is lost                |
|  6  | disconnected from a network           |

* `ap.status`:

|Value|              Meaning                  |
|:---:|---------------------------------------|
|  0  | inactive                              |
|  1  | active                                |

* `ap.lifetime`: time for which the AP will stay ON, after that it will be switched OFF

## Brightness control

**URI**: `/rest/bright`
**Method**: `GET` `POST`
### Success response
**Code**: `200`

#### Content example:
```json
{
	"minBright": 0.5,
	"maxBright": 1,
	"photoMin": 1000,
	"photoMax": 4095,
	"isFixed": true,
	"fixedBr": 0.1,
	"photoRead": 0,
	"currentBr": 1
}
```
where:
* `"minBright"` - `float` maximal allowed nixie brightness, ignored when `isFixed=true` (0-1)
* `"maxBright"` - `float` minimal allowed nixie brightness, ignored when `isFixed=true` (0-1)
* `"photoMin"` - `int` minimal reading of photo resistor ADC taken for brightness regulation, ignored when `isFixed=true` (0-4095)
* `"photoMax"` - `int` maximal reading of photo resistor ADC taken for brightness regulation, ignored when `isFixed=true` (0-4095)
* `"isFixed"` - `bool` control switch to set fixed or variable brightness
* `"fixedBr"` - `float` fixed nixie brightness, ignored when `isFixed=false` (0-1)
* `"photoRead"` - `int` raw photo resistor ADC reading (0-4095)
* `"currentBr"` - `float` current nixie brightness

### Error response
**Code**: `400`
#### Content example:
```json
{
	"message": "error description",
	"minBright": 0.5,
	"maxBright": 1,
	"photoMin": 1000,
	"photoMax": 4095,
	"isFixed": true,
	"fixedBr": 0.1,
	"photoRead": 0,
	"currentBr": 1
}
```
fields are the same as in case of normal response, additional `"message"` field is added with description of the failure.

### `POST` data example

```json
{
	"minBright": 0.5,
	"maxBright": 1,
	"photoMin": 1000,
	"photoMax": 4095,
	"isFixed": true,
	"fixedBr": 0.1
}
```
Fields descriptions are the same as in case of the normal response description. All the parameters will be stored in non-volatile memory on the clock. 