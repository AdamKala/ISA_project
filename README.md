# DNS Resolver
##### Adam Kala (xkalaa00)
##### Datum vytvoření: 19.11.2023
#### Hodnocení: 17/20
### Slovní komentář k hodnocení 

Dokumentace seznamuje čtenáře s problematikou a postupem řešení, ale bylo by dobré zlepšit strukturu
Nesprávná práce s bibliografií v dokumentaci
Informativní README
Dlouhé funkce, program je dlouhodobě nespravovatelný
Podařilo se úspěšně komunikovat pomocí DNS
Podpora autoritativní odpovědi
Podpora odpovědi od neautoritativního serveru
Úspěšně zobrazuje informace o nerekurzivním neautoritativním dotazu
Podpora AAAA
Chybí podpora PTR
Chybí podpora PTR6 (zkrácený zápis adresy)
Chybí podpora PTR6 (plný zápis adresy)
Podpora CNAME při překladu na IPv4
Podpora CNAME při překladu na IPv6
Ošetřená chyba nepřítomného serveru
Podpora serveru zadaného adresou IP
Podpora serveru zadaného adresou IPv6
Podpora serveru zadaného adresou IPv6
Úspešné zpracování zprávy bez odpovědi (sekce answer)

### Odevzdané soubory:
`manual.pdf`
`README`
`Makefile`
`dns.c`
`dns_test.py`

Cílem programu je zasílat dotazy na DNS servery podle informací, které získá z příkazového řádku a v čitelné podobě vypisovat přijaté odpovědi na standardní výstup.

## Použití:

Pořadí parametrů je libovolné, kromě -s, za kterým musí následovat adresa serveru.
```
./dns [-r] [-x] [-6] -s server [-p port] adresa
```
### Popis parametrů:

```-r```: Požadována rekurze (Recursion Desired = 1), jinak bez rekurze.
```-x```: Reverzní dotaz místo přímého.
```-6```: Dotaz typu AAAA místo výchozího A.
```-s```: IP adresa nebo doménové jméno serveru, kam se má zaslat dotaz.
```-p port```: Číslo portu, na který se má poslat dotaz, výchozí 53.
```adresa```: Dotazovaná adresa.

## Příklad spuštění a jeho výstupu:
### Spuštění 1:
```$ ./dns -r -s kazi.fit.vutbr.cz www.fit.vut.cz```

### Výstup 1:
```Authoritative: No, Recursive: Yes, Truncated: No
Question section (1)
  www.fit.vut.cz., A, IN
Answer section (1)
  www.fit.vut.cz., A, IN, 14400, 147.229.9.26
Authority section (0)
Additional section (0)
```

### Spuštění 2:
```$ ./dns -r -s kazi.fit.vutbr.cz www.github.com```

### Výstup 2:
```Authoritative: No, Recursive: Yes, Truncated: No
Question section (1)
  www.github.com., A, IN
Answer section (2)
  www.github.com., CNAME, IN, 3600, github.com.
  github.com., A, IN, 60, 140.82.121.3
Authority section (0)
Additional section (0)```
