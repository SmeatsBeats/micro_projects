# Write your code here :-)
# Complete project details at https://RandomNerdTutorials.com

from machine import Pin
from time import sleep

led = Pin(2, Pin.OUT)

while True:
  led.value(not led.value())
  sleep(0.5)# Write your code here :-)
