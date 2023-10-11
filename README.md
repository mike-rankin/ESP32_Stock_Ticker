# ESP32_Stock_Ticker
ESP32 based multiple stocks ticker
This project was created to monitor multiple stock and crypto prices throughout the day without having check on my phone. I've seen a few versions on Kickstarter that monitor just one at a time and wanted more. The sketch uses a free Finnhub acount using a loop and Json to cycle though grabbing the data for each stock ticker. Becuase of the free account limitations, updating every five minutes or so is fine.

With plenty of room left over on the board I placed a few sensors that I had on hand. Some being used and some for future use. The QwiiC connector is free to connect other i2c sensors and the ambient light sensor could be used to auto adjust the lcd backlight. On the software side, you could grab the time and update the stock prices more often only when the market is open.

![Image_0](https://github.com/mike-rankin/ESP32_Stock_Ticker/assets/4991664/78a8cb77-838e-40db-b3e7-2c8846187be2)
![Top](https://github.com/mike-rankin/ESP32_Stock_Ticker/assets/4991664/94079343-0a03-402c-bd2f-8c1e38d37d30)
![Bottom](https://github.com/mike-rankin/ESP32_Stock_Ticker/assets/4991664/66e79dfc-337e-4913-b6de-c97e3dd7df59)
