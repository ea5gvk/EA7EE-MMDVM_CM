# Description

This is the source code of several tools for cross-mode conversion for some digital voice protocols, based on Jonathan G4KLX's [MMDVM](https://github.com/g4klx) software.

In this fork I will only work on the YSF2DMR branch to obtain WiresX repeaters similar functionality. The idea is get network functionality right into the node instead into the Network. I will try to enhance the code to include new functionalities:

- Voice Beacon (done)
- AMBE recording (done)
- AMBE dynamic compression to avoid voice distortion on Yaesu equipment (done)
- Auto connect to last dynamic TG at start (done)
- Auto back to initial default TG on timeout (done)
- Bypass Brandmeister APRS functionality (done)
- Bypass Brandmeister two second Banning (working on it)
- Define Icon and message of Node sent to ARPS-IS (done)
- Use GM button from Yaesu Equipment to put repeating beacon directly to APRS-IS (done)
- Display number of nodes connected to master server TG on Wires-X connect info (done)
- Store WiresX SMS and Photo on Node for later retrieval (working on it)
- Send WiresX SMS to email and Telegram and back (working on it)
- Send WiresX Photo to email and Telegram and back (working on it)

Please be patient, I will upload my code on the repository as soon as possible and will mark those functions uploaded.

This software is licensed under the GPL v2 and is intended for amateur and educational use only. Use of this software for commercial purposes is strictly forbidden.

# How to get the source and compile the program right inside the Rasperry Pi pi-star distribution.

You can connect via SSH to the board running last pi-star version. Then you can download the source code and compile and install the updated YSF2DMR program. To do so, you can do:
```
cd /
sudo mkdir opt
sudo git clone https://github.com/msraya/MMDVM_CM.git
cd MMDVM_CM/YSF2DMR
make
sudo mv /usr/local/bin/YSF2DMR /usr/local/bin/YSF2DMR.old
sudo cp ./YSF2DMR /usr/local/bin/YSF2DMR
```
Then you should understand the ysf2dmr options explained below and go to edit the /etc/ysf2dmr file, and then reload the program:
```
systemctl restart ysf2dmr
```
Your can see the info in the log file /var/log/pi-star/YSF2DMR.....

# How to fully integrate YSF2DMR in Pi-Star image

Firstly, you must configure pi-star through the web UI to work with C4FM (YSFGateway) and YSF2DMR as usual. Remember that if you change anything directly in the mmdvmhost or ysf2dmr configuration files and then You change any setting on the web UI you will lost the manual file configuration changes. So I advice you only to use the PI-STAR Web UI at the very beginning.

Unfortunately, pi-star image relies on YSFGateway to make YSF2DMR work. So, you cannot use extended features of YSF2DMR as change TG and auto APRS GM Beacon. But don't worry, you can bypass YSFGateway easily so you can use YSF2DMR to the fullest potential.

To do so, you must edit the configuration file for mmdvmhost program. Go to edit /etc/mmdvmhost keying "sudo nano /etc/mmdvmhost" and change the two lines:
```
[System Fusion Network]
Enable=1
LocalAddress=127.0.0.1
LocalPort=3200   --->> change to 42000
GatewayAddress=127.0.0.1
GatewayPort=4200 --->> change to 42013
# ModeHang=3
Debug=0
```
Then, in the ysf2dmr configuration file, we will use these ports so both programs can communicate. Below I will comment all the ysf2dmr options with detail.

Also, you should disable all others modes as DMR or DSTAR so they do not start up in the bad moment.
In my image I also disable YSFGateway and YSFParrot with:
```
sudo systemctl stop YSFGateway
sudo systemctl disable YSFGateway
sudo systemctl stop YSFGateway.timer
sudo systemctl disable YSFGatway.timer
sudo systemctl stop YSFParrot
sudo systemctl disable YSFParrot
```
and also I advice you to rename program files under /usr/local/bin as YSFGateway and YSFParrot to YSFGateway.old and YSFParrot.old so they don't run by accident.  
```
sudo mv /usr/local/bin/YSFGateway /usr/local/bin/YSFGateway.old
sudo mv /usr/local/bin/YSFParrot /usr/local/bin/YSFParrot.old
``` 
Then you should restart the mmdvmhost program keying on the console:
``` 
sudo systemctl restart mmdvmhost
``` 
  
# How to improve the Pi-Star image

Pi-Star image is not optimized to support YSF2DMR. The first problem we need to fix is the periodically update of the DMR database so we don't need to be concerned about the obsolescence of the database. To do this we need to put in the /usr/local/bin folder the auxiliary file "updateDMRIDs.sh" so it can be called. Also we need to grant it execution permission. This file resides in the git folder MMDVM_CM/YSF2DMR/contrib.

We need to call this file periodically so it can update the database, we should modify the crontab system file so this update was done automatically by the Pi-Star image. 

Also, when using you Yaesu transceiver, you can ask for the TGList as do FTM-100, FTM-400 or FT-2 transceivers. However, to update the list directly from the Internet you must download it periodically. You must copy two files in /usr/local/bin. The files are "tg_generate.py" and "updateTGList.sh" both in the MMDVM_CM/YSF2DMR/contrib git folder. Don't forget to active execute permision. The TGList is different from the original TGList so it can include a field that inform us about the number of systems connected to a TG in a concrete Brandmeister Master Server. 

It only work for a specifically Master Server that is specified in the first line of the "updateTGList.sh" file:
``` 
wget -O /tmp/group.txt http://master.brandmeister.es/status/status.php
```
In my case I use the Spanish Master Server, but you can change the link to the master server status page. Bellow are the additional lines you will need to put in the crontab file so that it updates both the TGList and DMRId:
```
0 0,12  * * *   root    [ -x /usr/local/bin/updateDMRIDs.sh  ] && /usr/local/bin/updateDMRIDs.sh
0  *    * * *   root    [ -x /usr/local/bin/updateTGList.sh ] && /usr/local/bin/updateTGList.sh
```
Dont forget to put the TGList.txt file from /MMDVM_CM/YSF2DMR/crontrib folder to the /usr/local/etc folder so when you run the ysf2dmr program it get the proper version in place. You can also run the updateTGList.sh script to get the proper file in place.

I advice you also to include the DMR ID update at the end of /etc/rc.local file so that each time you start your HotSpot, it can source and update the last DMR database so you haven't any problems with callsigns.

# ysf2dmr configuration file explained

Now, we go to explain all the configurations possible in the ysf2dmr configuration file. On one hand, we have the LEGACY configuration options, that are the old options. On the other hand, we have the new options I added to the ysf2dmr configuration file that are marked with an asterisk [*].

## [Info]
This is where you put the general information about your HotSpot
### Latitude
This is Latitude information that using for the APRS subsystem. The APRS subsystem is included in YSF2DMR, so as it is impossible to disable Brandmeister APRS system to the best of my knowledge, we send a fake Latitude and we only have one icon on APRS. Example:
```
Latitude=37.8835
```
### Longitude
This is Longitude information that using for the APRS subsystem. The APRS subsystem is included in YSF2DMR, so as it is impossible to disable Brandmeister APRS system to the best of my knowledge, we send a fake Latitude and we only have one icon on APRS. Example:
```
Longitude=-6.778
```
### Location
This is the location of the HostSpot. This is information sent to the DMR Master Server. Example:
```
Location="Almonaster, IM67OG"
```
### Description
This is the description of the HostSpot. This is information sent to the DMR Master Server. Example:
```
Description="Sierra Huelva"
```
### URL
This is the URL of the owner of the HostSpot. This is information sent to the DMR Master Server so then you can clic on the URL under BrandMeister web page. Example:
```
URL=http://www.qrz.com/db/ea7rcc
```
### RXFrequency and TXFrequency
These are the HostSpot's TX and RX frequency, usually the same, but in simplex or duplex it can be different. Example:
```
RXFrequency=438100000
TXFrequency=438100000
```
### Power and Height
There are information sent to the DMR Master Server to show in web pages. Example:
```
Power=1
Height=0
```
##[YSF Network]
This section configures C4FM connection with the mmdvmhost program
### Suffix
You can add a suffix to the C4FM callsign show on the Yaesu Transceiver, but this option is not used in the updated YSF2DMR, so please leave it blank. Example:
```
Suffix=
```
### Callsign
This is the callsign that send to the C4FM equipment. Note that it include the suffix to be the same that DMR callsign as usual. Example:
```
Callsign=EA7RCC-3
```
### DstAddress and DstPort
These are the local host address and port where the mmdvmhost program is listen at, so both program can communicate. We use the same port in the mmdvmhost config file. Please don't change. Example:
```
DstAddress=127.0.0.1
DstPort=42000
```
### LocalAddress and LocalPort
These are the local host address and port where the mmdvmhost program is listen at, so both program can communicate. We use the same port in the mmdvmhost config file. Please don't change. Example:
```
LocalAddress=127.0.0.1
LocalPort=42013
```
### Daemon
Can take only two values (boolean), value "1" means that the ysf2dmr will work in the background, it's the usual way. You can use value "0" to execute the program in the foreground for debug purposes. Take into account that you must change the DisplayLevel and FileLevel options from the Debug section accordingly. Example:
```
Daemon=1
```
### EnableWiresX
This is an option that only take two values. value "1" means that you can change the TG using the Wires-X protocol. Also you can use other Wires-X commands to communicate with the HotSpot. Usually takes the value "1". Example:
```
EnableWiresX=1
```
 
## [DMR Network]
This section allows to configure DMR side options.
### StartupDstId
This is the start up TG and the main TG of the HotSpot. 

When the program start it ask to the master server for the last dynamic TG. If less that 15 minutes have passed, the Master Server will tell us which one is and we take this TG as the main TG. But if time passed, and the Master Server don't tell us which is the TG we are connected to (to the best of my knowledge, this is a BrandMeister limitation), we must change it to a know default TG. In this last case the program makes a change TG procedure, so we can absolutely sure that no other TGs are open in the BrandMeister network at the same time.

In this way we can ensure that only a TG on RX can be allow. This is critical for C4FM emulation as we can listen to several TGs at the same time without knowing it and it can lead to confusion. Example:
```
StartupDstId=2147
```
### Id
This is the Id we will use to register onto the BrandMeister Network. We can use 01, 02, 03 and so on as suffixes. Example:
```
Id=214753104
```
### StartupPC
This parameter can take two values. If the StartupDstId is a TG it will be "0", if the StartupDstId is private it will be "1". Example:
```
StartupPC=0
```
### Jitter
This parameter fix the receive buffer length, so please don't change it. Example:
```
Jitter=500
```
### EnableUnlink
This parameter can have two values. When it is "1" the program send unlink requests so only we will have one TG active at any moment. When it is "0" the unlink request don't will be sent. This is useful in the DMR+ network where no unlinks are used. But for BrandMeister it must be "1". Example:
```
EnableUnlink=1
```
### TGUnlink
This is the TG where unlink requests are sent to, usually 4000. Example:
```
TGUnlink=4000
```
### PCUnlink
This is the type of communication with the Unlink TG. "0" means TG request, "1" means private request. BrandMeister use TG request. Example:
```
PCUnlink=0
```
### Debug
This boolean parameter active more complete information in the log file for debug purposes. Example:
```
Debug=0
```
### TGListFile
This is the location of the TGList file that will be generated by periodic crontab task. You can leave as it is. Example:
```
TGListFile=/usr/local/etc/TGList.txt
```
### Local
This is the local port used for Brandmeister communication. Don't change. Example:
```
Local=62032
```
### Address, Password and Port
These are BrandMeister server options so we can connect trough the Internet and register our HotSpot. You should use your local server, for example in Spain I use the BrandMeister Spanish Master Server address and password. This is put automaticaly for you by the Pi-Star Web Interface. Example:
```
Address=84.232.5.113
Password="passw0rd"
Port=62031
```

##[DMR Id Lookup]
This section fix the location and update time for the master DMR ID Database. For C4FM this database is very important. The callsign will be extracted from the Id that Master Server send to us and then sent to the Yaesu transceiver in Callsign Format.
### File
This is the location of the database file. The crontab task will update this file in this location, so please don't change. Example:
```
File=/usr/local/etc/DMRIds.dat
```
### Time
This is the time period for the update of the database. It will be in hours, so you may want to update the database once or twice per day. Example:
```
Time=24
```

## [aprs.fi]
This section configures the APRS subsystem.
### AprsCallsign [*]
This is the APRS Callsign that will be sent to the APRS-IS server. You can add a suffix directly. Example:
```
AprsCallsign=EA7RCC-3
```
### HotSpotFollow [*]
This is a boolean option. With "1" the HostSpot location will be following the location of the station which have the same callsign as the HostSpot. In this way the HostSpot will be skipping near the owner station location. With "0" the HostSpot Icon will be static at the supplies coordinates. Mobile HotSpots use "1" value. Example:
```
HotSpotFollow=0
```
### Icon and text [*]
This is the icon and beacon text that will be sent to the ARPS-IS server. You can see it in the aprs.fi web page. Example:
```
Icon=YY
Beacon=YSF2DMR Public DMO
```
### BeaconTime [*]
This is the time period used for send the HotSpot Location Beacon to the APRS-IS Server in minutes. Usually it is 20 minutes. Example:
```
BeaconTime=20
```
### APIKey
This is the key to access APRS information trough the aprs.fi server. This is used to generate location info for all received DMR stations. So you can have the distance from you to the remote station right in your Yaesu transceiver display. For you to obtain this APIKey, you must register in the [aprs.fi sign up page] (https://aprs.fi/signup/). Registration is free. Once registered and replied to the registration email, you can get your APIKEy from the [aprs.fi my account page] (https://aprs.fi/account/) . Example:
```
APIKey=119415tK5xTsONKieXg3Mi
```
### Password
This is the APRS-IS Network Password. You can get it from any [password generator] (http://n5dux.com/ham/aprs-passcode/) . Example:
```
Password=12014
```
### Description
This description will be sent to the APRS-IS server for logging purposes. Example:
```
Description=EA7RCC_Pi-Star
```
### Enable
This switch enables the APRS subsystem. Can be "0" to disable APRS information or "1" to enable APRS. Example:
```
Enable=1
```
### Server and Port
This is the server and port for the APRS-IS Network. Usually you should use a server near you location. For example, I use one of the Spanish severs. This lines are configured for you from the Pi-Star web UI. It's important to remember that the server do not admit more than one session with the same station-ssid, so we need to add a suffix to the callsign or change the server.
```
Server=spain2.aprs2.net
Port=14580
```
### Refresh [*]
This is the time in minutes that the program wait to ask again for a callsign location, so we don't overload the aprs.fi server. Example:
```
Refresh=4
```

## [Storage] [*]
This section includes the new options for the new functionality of the program
### TimeoutTime [*]
This is the time in minutes that will last the TG when you change it using Wires-X commands. If you change TG using Wires-X keys with the Yaesu transceiver and then you don't transmit anything for this time period, the system will issue a change TG command automatically to return to the original TG when time expires. If you transmit the period will be reset. If you put TimeoutTime=0, the TG will remain forever. This TimeoutTime option is used in Public HotSpots to come back to the original TG. Example:
```
TimeoutTime=60
```
### BeaconTime [*]
This is the time period of the Voice Beacon. The voice beacon is an uncompressed AMBE file without header. We can generate an AMBE file with the SaveAMBE option below. It must reside in /usr/local/etc/beacon.amb . If the BeaconTime is zero or the file don't exist voice beacon will be disabled. Example:
```
BeaconTime=10
```
### SaveAMBE [*]
When this parameters value is "1" it allow us to save AMBE format files for voice coming from C4FM modulation. The files will be saved to the /tmp folder, their name will be a number counting up for each voice period. So we can create many beacon files and then try them. Example:
```
SaveAMBE=0
```
### TGReload [*]
This parameter specify the time period in minutes when an update of TGList will be done. This update re-reads the TGList.txt file. Example:
```
TGReload=60
```
### ABECompA and AMBECompB [*]
These are AMBE Voice Compression parameters. When DMR modulation is translated to C4FM the dynamic range of DMR modulation is greater than C4FM range (to the best of my knolodge it is because Yaesu have different audio adjust levels). This means that some DMR modulations will be severity distorted when listen on C4FM gear. 

This problem shows up when cheap DMR gear is used, the user modifies the microphone net o he/she use an external home made microphone. In adition, usualy the user screams over the microphone and the situation gets worse because usually DMR gear lacks audio compression. To avoid receiver distortion we unpack AMBE information and reduce the formant volume only for very distorted signals. We use a bi-linear table that we generate with two parameters. The formant volume has 32 levels. Parameter A set the limit level (32-CompA is maximun level). Parameter B set the start level of compression(32-CompA-CompB is start level). If any parameter is zero, the compression turns off.

We get other type of distortion that I found more pleasant for the human ear. Exact waveform recovery is not possible because the voice formants levels are modified and data is lost. 

It is important to note that this compression artifacts only takes place on overmodulated signals and it is more pleasant to the ear that listen at distorted signals. Example:
```
AMBECompA=4
AMBECompB=3
```
