<table width='100%'>
<tr>
<td width='75%' valign='top'>
<h1>Railuino</h1>

This library allows you to control your digital Märklin railway using <a href='http://www.arduino.cc'>Arduino</a>. It can be used in two flavors:<br>
<br>
<ul>
<li> As a Mobile Station 2. You can use a CAN shield such as <a href='http://www.watterott.com/de/Arduino-CANdiy-Shield'>this one</a> or <a href='https://www.sparkfun.com/products/10039'>this one</a> and a cable to connect your Arduino to the <a href='http://www.maerklin.de/de/service/suche/details.html?page=2&perpage=10&level1=2341&level2=2346&art_nr=60113&search=1&era=0&gaugechoice=0&groupchoice=0&subgroupchoice=0&catalogue=0&features=0&searchtext=anschlussbox&backlink=%2Fwww.maerklin.de%2Fde%2Fservice%2Fsuche%2Fproduktsuche.html'>Digital Connector Box</a> of a Mobile Station 2 and probably (untested) also to a Central Station 2. This allows you to control locomotives, functions and turnouts using MM2, DCC and other protocols. You can even read and write decoder CVs, as long as the decoder's protocol supports it (DCC does both, MM2 only write, others vary).<br>
</li>

<li> As an infrared controller. You can use a simple infrared LED and a resistor (as described <a href='http://www.arcfn.com/2009/08/multi-protocol-infrared-remote-library.html'>here</a>) to have your Arduino talk to the very basic IR receiver box that comes with various starter packages. This allows you to control four MM2 locomotives on fixed addresses (the classic Delta addresses). You can also control sixteen turnouts (this is an undocumented feature of the IR box, and the original Marklin IR controller doesn't support it).<br>
</li>

</ul>

In addition to controlling things on and connected to the track the library allows you to receive reports about track usage using the standard <a href='http://www.digital-bahn.de/info_tech/s88.htm'>S88 bus</a>.<br>
<br>
Installation is easy: Just get the latest release from the downloads page and place the contents of the "src" directory in a<br>
"Railuino" directory under your Arduino "libraries" directory. Then restart Arduino. You should now see a bunch of new examples that teach you how to use Railuino. The "Misc/Tests" example is a good way of validating your setup.<br>
<br>
For documentation on the functions I currently recommend to read the comments in the "Railuino.h" header file. There are also several sets of slides on the downloads page that describe the overall approach and the hardware. Finally, there is a video from <a href='http://www.youtube.com/watch?v=RnU9YFDxkqg'>LinuxTag</a> and another one from <a href='http://www.youtube.com/watch?v=5UmcGqWM1BM'>DroidConNL</a> on YouTube.<br>
<br>
The library itself is made available under the GNU Lesser General Public License (LGPL). See the LICENSE file for details. All examples except the test suite are licensed under the Creative Commons Zero license, which effectively makes them public domain. The test suite is also licensed under the LGPL.<br>
<br>
Have fun!<br>
</td>
<td width='25%' valign='top'>
<h1>News</h1>
<ul>

<li>2013-10-05: Looking for a PC front-end to Railuino? Try Yasaan's <a href='http://yaasan.mints.ne.jp/index.html'>Desktop Station</a> for Windows. Very cool!</li>

<li>2013-08-03: Railuino featured at <a href='http://de.amiando.com/makerfaire2013.html'>Maker Faire Hannover</a>, slides available online.</li>

<li>2013-03-06: There is now a little Railuino spin-off for hacking your car. It's called <a href='http://code.google.com/p/mechanic/'>Mechanic</a>.</li>

<li>2013-03-06: Errata c't Hardware Hacks article. Unfortunately there is an error in the cable pinout. The correct pinout looks like this:<br>
<br>
<img src='http://www.pleumann.de/misc/railuino-kabel-korrektur.png' width='100%' />

Also, the Mini-DIN pin numbering is to be read from the soldering side, which may not be totally clear given the picture. Sorry!<br>
</li>

<li>2013-02-23: Railuino version 0.9 available for download. Among the improvements is compatibility with Arduino <a href='http://arduino.cc/en/Main/ArduinoBoardLeonardo'>Leonardo</a>.<br>
</li>

<li>2013-02-23: Railuino featured in German <a href='http://shop.heise.de/katalog/ct-hardware-hacks-1-2013'>c't Hardware Hacks</a> magazine. This article should give a good introduction to the project.<br>
</li>

<li>2013-02-23: New CANdiy shield for Arduino developed by myself and turned into a professional product by <a href='http://www.watterott.com/de/Arduino-CANdiy-Shield'>Watterott</a>.<br>
</li>
</ul>
</td>

</tr>
</table>