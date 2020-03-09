# ClientMigration

## Specialiseringsbeskrivning: 

Målet är att bygga upp en klient och serverstruktur med möjlighet för att migrera klienter mellan servarinstanser i prestanda balanseringssyfte.

Detta genom att en klient kopplar upp sig till en känd server i ’router’ konfiguration som skickar vidare klienten till en så obelastad och passande server som möjligt baserat på en kombination av klientens ping, serverns nuvarande belastning, och spelarens position i spelvärlden.

Vid behov av avbelastning från servern så ska den kunna skicka ’tillbaka’ klienter till ’router’ servern som migrerar dem till en mindre belastad server. Klienten ska uppleva denna transport så lite som möjligt.

För att avbelasta servarna ska det också gå att dynamiskt koppla upp nya servar till ’router’ servern som efter behov tar på sig klienter från överbelastade servrar.

Om tid och möjlighet finns vill jag också etablera ett system varvid nya ’router’ servrar går att dynamiskt kopplas till varandra för en skalbar kompositstruktur.

Om även mer tid finns Vill jag koppla servrarnas uppmätta ’hälsa’ och belastning till en statuspanel på min hemsida och om möjligt koppla upp det med [Grafana](https://grafana.com/) och [Graphite](https://grafana.com/oss/graphite/) för att få [attraktiva paneler](https://i.imgur.com/ElSzKXj.png).
