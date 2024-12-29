Am inceput implementarea acestei teme prin citirea fisierului preluat din argumente. Am salvat numele fiecarui fisier intr-un
vector. Am pornit toate cele M + R thread-uri de la inceput, primelor M thread-uri le-am dat ca argument functia 
thread_loop_function, iar ultimelor R, functia "functie". Am facut si o coada de task-uri, in care am pus ca task functia
map, avand ca argument o structura de date ce contine numarul fisierului, numele, si map-ul in care vor fi puse cuvintele
din acest fisier. Fiecare mapper preia cate un task pana cand lista e goala, iar, la finalul fiecarui task, adauga lista cu
cuvinte intr-o lista de liste, regasita in structura de date create pentru threadpool. Pentru lista fiecarui mapper, cuvintele
sunt adaugate unul dupa altul, fara dubluri. Intre timp, thread-urile reduce asteapta semnalul sa isi poate continua treaba.
Atunci cand s-a ajuns la ultimul mapper, se adauga in lista de task-uri functia reduce pentru fiecare litera de la a la z.
Se ajunge la bariera si se da semnal catre reducers ca pot prelua task-uri din lista. Atunci cand un reducer preia un task,
intra in functia reduce si cauta in lista de liste toate cuvintele incepand cu litera sa. Adaugarea in lista noua se face
in mod alfabetic, iar atunci cand un cuvant a fost deja adaugat in lista, i se adauga numarul fisierului in lista de fisiere,
regasita pentru fiecare cuvant in parte. Dupa aceasta parte, reducer-ul creeaza o lista noua, in care adauga toate elementele
din lista sortata alfabetic, doar ca de aceasta data, le sorteaza si dupa numarul total de fisiere in care se regaseste fiecare
cuvant. La finalul functiei reduce, thread-ul creeaza fisierul "litera".txt, si isi printeaza lista sortata.