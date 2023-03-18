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
### Data example
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