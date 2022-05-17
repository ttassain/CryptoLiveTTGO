# CryptoLiveTTGO

Petit programme qui permet de voir l'évolution de vos cryptos monnaies.

## Hardware

Module TTGO T-Display (ESP-32 avec ecran TFT ST7789V 135x240)

## Affichage

Cours des monnaies :\
![price](https://github.com/ttassain/CryptoLiveTTGO/blob/main/blob/price.jpg?raw=true)

Graph d'évolution :\
![graph](https://github.com/ttassain/CryptoLiveTTGO/blob/main/blob/graph.jpg?raw=true)

Nombre de coins :\
![Coin](https://github.com/ttassain/CryptoLiveTTGO/blob/main/blob/coin.jpg?raw=true)

Valeur de mes cryptos:\
oui... je suis pas riche, merci de faire un transfert si ça déborde chez vous vers :\
ETH : 0x257fFb03e617f9c9ebB2e719b4775F85F12c6567\
BTC : 1DeuttaiHr4aGswVszizMYYzDUAVmex63q\
![money](https://github.com/ttassain/CryptoLiveTTGO/blob/main/blob/money.jpg?raw=true)

Charge de la batterie :\
![about](https://github.com/ttassain/CryptoLiveTTGO/blob/main/blob/about.jpg?raw=true)

## Api utilisées

### Etherscan

https://docs.etherscan.io/api-endpoints/accounts

### Ergoplatform

https://api.ergoplatform.com/api/v1/docs/#operation/getApiV1AddressesP1BalanceConfirmed

### Blockchain

https://www.blockchain.com/api/blockchain_api

### Cryptocompare

https://min-api.cryptocompare.com/documentation?key=Price&cat=multipleSymbolsPriceEndpoint

## Boutons

| GPIO | Bouton | Action |
| --- | --- | ----------- |
| 35 | UP | Swap screen page (coin, money,...)|
| 0 | DOWN | Refresh current screen|

## Author

TASSAIN Thierry

## License

CryptoLiveTTGO is licensed under the Apache v2 License. See the LICENSE file for more info.