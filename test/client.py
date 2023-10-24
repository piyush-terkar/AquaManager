import sys
import json
import requests
import time
from requests.packages.urllib3.exceptions import InsecureRequestWarning
requests.packages.urllib3.disable_warnings(InsecureRequestWarning)

args = sys.argv[1].strip()

URL = "https://" + args + "/sync"
print(URL)

while True:
    r = requests.get(URL, verify=False)
    data = r.json()
    timeStamp = time.strftime("%H:%M:%S", time.gmtime())
    print(f'[{timeStamp}]: {data}')
    time.sleep(1)