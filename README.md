# ClientMigration

## Specialiseringsbeskrivning: 

Målet är att bygga upp en klient och serverstruktur med möjlighet för att migrera klienter mellan servarinstanser i prestanda balanseringssyfte.

Detta genom att en klient kopplar upp sig till en känd server i ’router’ konfiguration som skickar vidare klienten till en så obelastad och passande server som möjligt baserat på en kombination av klientens ping, serverns nuvarande belastning, och spelarens position i spelvärlden.

Vid behov av avbelastning från servern så ska den kunna skicka ’tillbaka’ klienter till ’router’ servern som migrerar dem till en mindre belastad server. Klienten ska uppleva denna transport så lite som möjligt.

För att avbelasta servarna ska det också gå att dynamiskt koppla upp nya servar till ’router’ servern som efter behov tar på sig klienter från överbelastade servrar.

Om tid och möjlighet finns vill jag också etablera ett system varvid nya ’router’ servrar går att dynamiskt kopplas till varandra för en skalbar kompositstruktur.

Om även mer tid finns Vill jag koppla servrarnas uppmätta ’hälsa’ och belastning till en statuspanel på min hemsida och om möjligt koppla upp det med [Grafana](https://grafana.com/) och [Graphite](https://grafana.com/oss/graphite/) för att få [attraktiva paneler](https://i.imgur.com/ElSzKXj.png).

## Tidsplanering: 

1.	09/03 måndag:	Planera upp specialiseringen

2.	10/03 tisdag:	Rensa upp nuvarande projekt och extrahera simpel klient-server

3.	11/03 onsdag:	Få klient att koppla upp sig till servern baserad på en känd ip

4.	12/03 torsdag:	Få tag i en domän och börja undersöka vad fanken dns är


5.	16/03 måndag:	om dns är simpelt nog, koppla domänen till min hemaddress annars hitta ett webhotell el.dyl för att hosta en simpel redirectsida och med en kombination av LibCurl och string parsing göra det som jag tror dns gör automagiskt dvs redirecta till min hemadress

6.	17/03 tisdag:	Få klienten att koppla upp sig till ett domännamn och få en koppling till en server som kör på min hemdator

7.	18/03 onsdag:	 Sätta upp en simpel redirekt server och koppla upp klienten via den istället

8.	19/03 torsdag:	Buggfixing/bufferdag ’Vad är det här!?’, ’Vad gjorde jag här!?’, ’Varför gjorde jag så!?’ eller liknande kommentarer kan förekomma

9.	23/03 måndag:	Utöka redirekt servern så den kan ha flera ’spel’servrar uppkopplade och välja mellan dem.

10.	24/03 tisdag:	etablerara ett sätt för ’spel’serverna att rapportera sin status till ’router’ servern och ett sätt för klienter att

generera en låtsas ’load’, filöverföring data subscription el.dyl

11.	25/03 onsdag:	Kopiera en klients data från en server till en annan via ’router’ servern och sen låta en klient koppla upp sig till 

den andra servern med redan etablerad data

12.	26/03 torsdag:	Få en server att migrera en klient genom att först skicka dens data sen be den koppla om sig till en annan server



13.	30/04 måndag:	Buggfixing/bufferdag

14.	31/04 tisdag:	Få servar att dela belastning om en ny server kopplar upp sig

15.	01/04 onsdag:	Bufferdag för Komposit routing eller Hemsida/portfolio

16.	02/04 torsdag:	Bufferdag för Komposit routing eller Hemsida/portfolio


17.	06/04 måndag:	Bufferdag för Komposit routing eller Hemsida/portfolio

18.	07/04 tisdag:	Bufferdag för Grafana eller Hemsida/portfolio

19.	08/04 onsdag:	Bufferdag för Grafana eller Hemsida/portfolio

20.	09/04 torsdag:	Bufferdag för Grafana eller Hemsida/portfolio

