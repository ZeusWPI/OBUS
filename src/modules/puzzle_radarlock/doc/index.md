# radarlock

## Principe
Radarlock werkt als een vergrendelingssysteem dat kan worden ontgrendeld door objecten onder de juiste hoek en op de juiste afstand van de sensor te plaatsen.

De 'n' richtingen kunnen worden vergeleken met een wachtwoord van 'n' karakters lang. 
en de afstanden binnen deze richtingen kunnen worden vergeleken met de mogelijke karakters dat kunnen ingevuld worden.

De radarsensor controleert drie richtingen(hoeken) voor voorwerpen binnen een bepaald bereik van de sensor. Als er een voorwerp binnen dit bereik is, zal worden gecontroleerd of deze afstand overeenkomt met de vereiste afstand voor deze richting. Zodra de afstand voor elke richting gelijk is aan de afstand die vereist is voor de corresponderende richting, zal de puzzel als opgelost worden beschouwd wanneer de gebruiker op de verzendknop drukt.

Als de gebruiker op de verzendknop drukt terwijl een van de afstanden ongeldig is, dan krijgt de gebruiker een strike.  

## Taak van de helper:

De helper zal de afstanden met de corresponderende richting aan de gebruiker moeten doorgeven.   

Aan de hand van de onderstaande *puzzelachtige* afbeeldingen moet de helper in staat zijn de **juiste** afstanden te geven onder de **juiste** hoek 
en onder welke omstandigheden de ledjes branden.


|   |   |
|--------------------------------|----------------|
|   |   |
| ![](./puzzle_radarlock/red_45.png){height=15%} ![](./puzzle_radarlock/red_90.png){height=15%} ![](./puzzle_radarlock/red_135.png){height=15%} | 28 cm |
|   |   |
| ![](./puzzle_radarlock/green_45.png){height=15%} ![](./puzzle_radarlock/green_90.png){height=15%} ![](./puzzle_radarlock/green_135.png){height=15%} | 10 cm |
|   |   |
| ![](./puzzle_radarlock/yellow_45.png){height=15%} ![](./puzzle_radarlock/yellow_90.png){height=15%} ![](./puzzle_radarlock/yellow_135.png){height=15%} | 44 cm | 
|   |   |
| ![](./puzzle_radarlock/empty_45.png){height=15%} ![](./puzzle_radarlock/empty_90.png){height=15%} ![](./puzzle_radarlock/empty_135.png){height=15%} |       |
