# Tema 4 PCOM *Cojocaru Andrei 325CD*

## Flow-ul aplicatiei e cam in felul urmator :

Verific daca userul a introdus un input valid, dupa care performez
requesturile catre server in functie de comanda, daca a fost o comanda valida
de login setez un cookie de sesiune. Dupa, daca a fost o comanda valida de 
enter_library setez tokenul pentru library. Dupa in functie de validitatea 
celor doua fac alte actiuni de genul get_book, add_book. Pentru logout
resetez tokenul de autentificare si librarie. Toate astea verificand 
integritatea inputul de la user pentru comenzile ce necesita acest lucru. 

## Am implementat toate functionalitatile aplicatiei dupa cum urmeaza:

1. `Login` - primesc input de la user si trimit requestul specific 
pentru login la server, dupa ce primesc raspuns verific daca 
am primit o eroare sau nu, si daca primesc eroare fac display
la un output cu eroare, in caz contrar parsez cookie de sesiune
(extractand stringul exact dupa Set-Cookie pana la primul spatiu din raspuns)
cu care urmeaza sa operez cu alte functionalitati precum `enter_library`.
De asemenea verific daca nu cumva user-ul a dat login deja.

2. `Register` - iarasi in functie de input trimit request la server 
si daca primesc ca raspuns de la server o eroare afisez eroarea respectiva, 
daca nu userul poate continua sa introduca comenzi.

3. `Enter_library` - mai intai verific daca userul este logat, adica daca 
exista un cookie de sesiune si daca da, generez requestul si il 
trimit la server. Daca primesc un raspuns de succes de la server parsez
si pastrez tokenul pentru library for further use, daca e eroare 
afisez output specific. De asemenea verific daca userul nu a intrat deja 
in librarie.

4. `Get_books` - mai intai verific daca exista un token de librarie si 
daca user-ul este logat, in caz afirmativ trimit requestul la server
si parsez raspunsul gasind primul caracter "[" , astfel incat sa 
afisez cartile asa cum le trimite serverul. 

5. `Get_book` - iarasi, verific daca exista un token de librarie valid 
si daca userul e logat, in caz contrar afisez output de eroare. In caz 
afirmativ - primesc input de la user pentru id si trimit requestul 
catre server . Verific daca am primit un raspuns de succes si daca da,
parsez raspunsul si afisez jsonul cu cartea returnata, daca nu afisez 
eroare ca idul nu e corect.

6. `Add_book` - same cu verificatul tokenului pentru autentificare si 
librarie. Primesc inputul de la user si validez daca am un input valid
pentru field-urile cartii. Generez requestul si il trimit la server si 
daca serverul intoarce succes sau eroare afisez outputul respectiv. Am 
adaugat o verificare noua pentru a nu avea numere in numele genului.

7. `Delete_book` - again, verific tokenul de autentificare si librarie.
Primesc inputul pentru id si trimit requestul la server. Daca returneaza 
eroare(eroarea se afiseaza daca id-ul nu a fost gasit) sau succes afisez
output respectiv. 

8. `Logout` - am folosit doua functii. Prima functie verifica daca avem 
un user logat la un moment de timp, daca da anuleaza cookie de sesiune,
iar a doua functie anuleaza token-ul pentru librarie. De asemenea verific 
daca userul este logat deja. 

## Pentru json am folosit libraria nhlomann pentru c++:
https://github.com/nlohmann/json

Am folosit functiile:

1. `basic_extract_json_response` pentru a scoate orice raspuns care are format de json din raspunsul serverului.

2. `obiect_json.dump()` - pentru a face display la obiectul 
json extractat.

3. am folosit `obiect_json["field"]=value` pentru a genera ca json 
payloadul trimis catre server.

4. am folosit `json::parse(string exemplu)` pentru a transforma 
un string in obiect json dupa caz. 