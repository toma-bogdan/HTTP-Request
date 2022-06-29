//Toma Bogdan-Nicolae
//323CB

/*
    Clientul interactioneaza cu serverul web si poate efectua urmatoarele comenzi:
poate sa se inregistreze/logheze, sa obtina acces la biblioteca, sa adauge/stearga
o carte, sa obtina detalii despre toate cartile sau o anumita carte.Pentru oricare
comanda se deschide o conexiune noua cu serverul dat, printr-un socket.

    Clientul va trimite diferite request-uri catre server, in functie de comanda dorita
(GET, POST, DELETE) si va astepta raspunsul serverului pentru a printa posibilele erori
sau daca operatia a fost efectuata cu succes. Pentru a trimite informatii catre server
vom folosi format json, astfel ne vom ajuta de biblioteca parson.

    Din biblioteca parson vom avea 2 obiecte, "root_val" care este initializat prin
functia init_object si reprezinta obiectul json propriu-zis, si root_obj prin care
se adauga campuri la root_val. Dupa ce se contruieste obiectul json, in convertim
inapoi in string prin functia serialize_to_string; 

    Pentru request-uri, se formeaza un string in care se pune pe prima linie numele
(DELETE, POST, GET), url-ul(calea) si versiunea de http. Pe a doua linie se adauga
host-ul, urmat de cookie-uri si header-ul Authorization, unde se afla JWT. Pentru
cererea POST, la final se adauga si continutul, retinandu-se tipul si lungimea 
acestuia.

        REGISTER
    Se citesc username-ul si parola noului utilizator si se construieste obiectul json
si se trimite un request de tip POST catre server cu acesta. In functie de statusul
prmit de la server printam raspunsul (201 - OK, 400 - username-ul este deja folosit). 

        LOGIN
    Similar ca la register, se creaza obietul json si se trimite un request POST catre
server, insa acum trebuie parcurs raspunsul serverului, pentru a se retine cookie-ul
intors de server sau eroarea returnata. De asemenea, avem 2 tipuri de erori intoarse 
(username nu existe sau parola gresita) .

        ENTER_LIBRARY
    Se trimite un request GET catre server, care returneaza eroare daca user-ul nu
este logat, sau trimite token-ul JWT. Astfel, daca statusul returnat este 401 se
prnteaza eroarea, altfel se parcurge raspunsul pentru a retine token-ul JWT.

        GET_BOOKS
    Se trimite un GET request, iar daca nu exista token-ul JWT, sau este invalid
se afiseaza eroare, altfel se primeste lista de carti de la server si se afiseaza

        GET_BOOK
    Prima data este verificat JWT-ul, iar daca acesta este valid se citeste id-ul
cartii dorite si se ataseaza la url, dupa care se trimite un GET request cu acesta.
Daca id-ul este valid serverul va trimite informatiile despre carte care vor fi
afisate, altfel se va afisa eroare.

        ADD_BOOK
    Precum la comenzile de mai sus se verifica JWT, apoi se creaza un obiect json
cu campurile title, author, genre, page_count, publisher si se verifica daca
acestea sunt valide. In caz afirmativ, se trimite un request POST cu obiectul creat
si se afiseaza un mesaj de succes.

        DELETE_BOOK
    Similar ca la add_book se contruieste url-ul in functie de id-ul folosit, insa
se trimite un Delete request catre server si se afiseaza raspunsul serverului 
(eroare/succes).

*/