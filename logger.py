import requests
import time



while True:
	with open('log.txt', 'a') as f:
		response = requests.get('http://10.42.0.139/rest/temperature')
		temp=float(response.json()["temperature"])
		print(temp)
		f.write(f"{temp}\n")
	time.sleep(10)

