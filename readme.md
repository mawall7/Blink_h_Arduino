IoT25 Individuell inlämningsuppgift Blink-hell 13 Feb 2026

(Här kan beskrivning av kommandon läggas till)

Kör programmet i WOKWI > genom att trycka på Run

Uppgift:

1. Vi har att röda, gröna, blå och vita lysdioderna ska blinka på och av med en bastid på 0.25 sekunders tidsfördröjning (alla ska blinka tillsammans och inte individuellt)! Inga Busy-Wait-lösningar utan använd millis() osv.

2. En av potentiometrarna, den som är kopplad till A0, ska kunna modifiera tiden för blinkningen ovan. När potentiometern är på 0 ska ingen tid läggas till. När den är på max, dvs 1023, så ska 2.55 sekunder läggas på så att det blir totalt 2.80 sekunders fördröjning mellan blinkningarna.
   Obs omvandlingen 0–1023 → 0–255!

3. Det finns en rotationsenkoder/rotationsgivare som ska kontrollera RGB-lysdioden. Vid start ska RGB-lampan vara släckt. När man roterar rotationsgivaren medurs ska den växla mellan färgerna i följande ordning:
   Röd – Grön – Blå – Alla färger på (vit) – Släckt!
   När man roterar den moturs går vi såklart baklänges.

4. Rotationsenkodern har en knapp också som aktiveras när den trycks ner. Den ska agera som en ”select/enable” vid aktivering. Är någon av de fyra färgerna ovan valda så ska RGB-lampan börja blinka för att signalera att den färgen är vald och aktiv. Trycker vi ner knappen igen så avaktiverar vi ”select/enable” och slutar blinka. Vi ska inte kunna rotera och byta färg när vi är i aktivt läge. Se nedan för mer instruktioner.

5. Det finns 2 knappar. En av knapparna ska, givet att en färg är aktiverad som beskrivet ovan, avbryta den lysdiod med samma färg från att blinka och gå in i ett toggle-state. Dvs den kan tändas och släckas med hjälp av knappen så länge den färgen är vald. Kom ihåg att den stannar i det tillståndet tills nedan kriterier är uppfyllda!

6. Den andra knappen ska göra en reset på den färg som är aktiverad och göra så att den hamnar i blink-mode igen (men endast om den är aktiverad med hjälp av rotationsgivaren såklart!).

OBS: Alla knappar ska vara avstudsade! Dvs en mjukvarulösning för att undvika felavläsning på grund av ringningar när vi öppnar och sluter brytare! Ingen Busy-Wait-lösning här heller!!

TODO

Om man trycker ner rotationsgivarens knapp när ingen av färgerna är valda ska blinkningen av de vanliga lysdioderna upphöra och en annan typ av blinkning påbörja. Vi ska då tända och släcka varje individuell lysdiod i följande ordning:
Vit – Röd – Grön – Blå och sedan om och om igen. Dvs vi stegar runt kan man tänka. Vi börjar med att en lampa är tänd i 25 millisekunder. När knappen på rotationsgivaren trycks igen återgår vi till vanlig blink!

Om vi är i blinkstadiet ovan så ska potentiometer 2, den som finns på A1, styra blinkhastigheten. Vi har att för varje värde hos potentiometern, 0–1023, så plussar vi på värdet som antal millisekunder.

Det ska också finnas möjlighet att styra från en terminal via UART med hjälp av olika kommandon! Det som ska finnas med är:

• Disable color där color är någon av färgerna och det kommandot gör är att den stänger av den färgen att blinka vid vanliga blinken.

• Toggle color togglar mellan på och av för färgen givet att den är disable.

• Enable color ansätter den färgen att börja blinka igen.


