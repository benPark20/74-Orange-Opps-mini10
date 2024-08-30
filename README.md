# 74-Orange-Opps

## We are Team 74 of mini FRC, the Orange Opps.

![Shooting Speaker](https://cloud-5xvvppbxg-hack-club-bot.vercel.app/0img_1368.mp4)

Our rookie season was 2024 and it was great! You can watch [the stream of the event](https://www.youtube.com/watch?v=baMjMV4FzJ8) but it is really long so i would reccomend if you want to see to watch [match 17](https://youtu.be/baMjMV4FzJ8?t=8170) (*we are the middle one on blue alliance*). Here are some of our accomplishments at Mini FRC 10:
- Our alliance made it to the semifinals in the losers bracket in a double elimintaion tournement (*That's 3rd Place!*),
- We managed to be the first, and one of the only, team to successfully climb on the stage,
- and we had the cleanest looking robot ~~*biased*~~.


## Design

We designed our robot heavily off of the robot that our team's team had competed and made worlds with for regular, full-sized FRC. Our workflow was in OnShape so you can [easily view and download/3D print CAD here](https://cad.onshape.com/documents/561065ee11688eb6314d06ba/w/829a4e28d8331633d6eed549/e/7dac9a85c5c0d4fed6ff8528?renderMode=0&uiState=66d23f52e866a03951123aa6).

![Robot CAD](https://cloud-72e9gzrtb-hack-club-bot.vercel.app/0master_assembly-2.png)
*pretend that those are mechanum wheels (it's a pain to deal with them in cad)*

## Assembly

Assembly of the robot was fairly straight forward since we CAD to go off of, but we did need a lot of parts and a lot of testing ![testing shooter](https://cloud-c3s2tiukz-hack-club-bot.vercel.app/0img_1355.mp4).
This is a quick BOM, the total cost adds up to around $70 and it was split between the four people on our team
| Item              | Quantity |
| :---------------- | :------: |
| ESP32             |    1     |
| Alfredo NoU2      |    1     |
| [9V Battery](https://www.alfredosys.com/news/fun-facts-about-batteries/)*       |    1     |
| Mechanum Wheels   |    4     |
| TT Motor + gearbox|    4     |
| TT Motor          |    2     |
| n20 Motor         |    2     |
| 50g servo         |    1     |
| Grip Tape         |    1     |
| Wires             |    1     |


## Code

Our code is based in Arduino IDE and runs on an ESP32 connected to a [NoU2 board](https://www.alfredosys.com/products/alfredo-nou2/) though we have plans to upgrade to a NoU3 when it comes out. I have been working with the code to implement an ultrasonic sensor that can detect how far away our robot is from the stage so that it can automatically aim and shoot from any point on our wing with no obstruction.


## How can you do the same

This was only possible with the help of [Alfredo Systems](https://www.alfredosys.com) and the wonderful program that they are running. I you want to just buy parts and make your own then go for it, but if you are wanting to hold a competition for you and your community then reach out to them on their website! They provide all the resources you would ever need to hold one of these. It is a great outreach opportunity and really a fun event all around.



