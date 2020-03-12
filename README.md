# ClientMigration

## Specialiseringsbeskrivning: 

Målet är att bygga upp en klient och serverstruktur med möjlighet för att migrera klienter mellan servarinstanser i prestanda balanseringssyfte.

Detta genom att en klient kopplar upp sig till en känd server i ’router’ konfiguration som skickar vidare klienten till en så obelastad och passande server som möjligt baserat på en kombination av klientens ping, serverns nuvarande belastning, och spelarens position i spelvärlden.

Vid behov av avbelastning från servern så ska den kunna skicka ’tillbaka’ klienter till ’router’ servern som migrerar dem till en mindre belastad server. Klienten ska uppleva denna transport så lite som möjligt.

För att avbelasta servarna ska det också gå att dynamiskt koppla upp nya servar till ’router’ servern som efter behov tar på sig klienter från överbelastade servrar.

Om tid och möjlighet finns vill jag också etablera ett system varvid nya ’router’ servrar går att dynamiskt kopplas till varandra för en skalbar kompositstruktur.

Om även mer tid finns Vill jag koppla servrarnas uppmätta ’hälsa’ och belastning till en statuspanel på min hemsida och om möjligt koppla upp det med [Grafana](https://grafana.com/) och [Graphite](https://grafana.com/oss/graphite/) för att få [attraktiva paneler](https://i.imgur.com/ElSzKXj.png).

## Tidsplanering: 

~__09/03 måndag:__	Planera upp specialiseringen~

~__10/03 tisdag:__	Rensa upp nuvarande projekt och extrahera simpel klient-server~

~__11/03 onsdag:__	Få klient att koppla upp sig till servern baserad på en känd ip~

~__12/03 torsdag:__	Få tag i en domän och börja undersöka vad fanken dns är~

__16/03 måndag:__	~om dns är simpelt nog, koppla domänen till min hemaddress annars hitta ett webhotell el.dyl för att hosta en simpel redirectsida och med en kombination av LibCurl och string parsing göra det som jag tror dns gör automagiskt dvs redirecta till min hemadress~

__17/03 tisdag:__	~Få klienten att koppla upp sig till ett domännamn och få en koppling till en server som kör på min hemdator~

__18/03 onsdag:__	 Sätta upp en simpel redirekt server och koppla upp klienten via den istället

__19/03 torsdag:__	Buggfixing/bufferdag ’Vad är det här!?’, ’Vad gjorde jag här!?’, ’Varför gjorde jag så!?’ eller liknande kommentarer kan förekomma

__23/03 måndag:__	Utöka redirekt servern så den kan ha flera ’spel’servrar uppkopplade och välja mellan dem.

__24/03 tisdag:__	etablerara ett sätt för ’spel’serverna att rapportera sin status till ’router’ servern och ett sätt för klienter att generera en låtsas ’load’, filöverföring data subscription el.dyl

__25/03 onsdag:__	Kopiera en klients data från en server till en annan via ’router’ servern och sen låta en klient koppla upp sig till den andra servern med redan etablerad data

__26/03 torsdag:__	Få en server att migrera en klient genom att först skicka dens data sen be den koppla om sig till en annan server

__30/04 måndag:__	Buggfixing/bufferdag

__31/04 tisdag:__	Få servar att dela belastning om en ny server kopplar upp sig

__01/04 onsdag:__	Bufferdag för Komposit routing eller Hemsida/portfolio

__02/04 torsdag:__	Bufferdag för Komposit routing eller Hemsida/portfolio

__06/04 måndag:__	Bufferdag för Komposit routing eller Hemsida/portfolio

__07/04 tisdag:__	Bufferdag för Grafana eller Hemsida/portfolio

__08/04 onsdag:__	Bufferdag för Grafana eller Hemsida/portfolio

__09/04 torsdag:__	Bufferdag för Grafana eller Hemsida/portfolio

