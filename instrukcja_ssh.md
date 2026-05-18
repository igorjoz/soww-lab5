# Instrukcja uruchamiania klastrowego (MPI over SSH)

Aby uruchomić aplikację MPI równolegle na wielu maszynach (np. w sali laboratoryjnej) komunikujących się przez protokół SSH, postępuj zgodnie z poniższymi krokami.

## 1. Konfiguracja logowania bezhasłowego (SSH)

Interfejs MPI (często pod spodem używający menedżera procesów ORTED) musi mieć możliwość zdalnego uruchamiania procesów roboczych bez czekania na hasło. Konieczne jest wygenerowanie i rozesłanie kluczy SSH. 

Kroki:
1. Zaloguj się na główną stację w laboratorium (np. wpisując \`ssh lab@glowna-maszyna\`).
2. W terminalu wygeneruj klucz RSA / ED25519 (jeśli zapulsuje o podanie hasła/"passphrase", pozostaw puste naciskając ENTER):
   \`\`\`bash
   ssh-keygen -t ed25519
   \`\`\`
3. Skopiuj klucz publiczny na każdą stację roboczą, na której będziesz prowadził obliczenia. Do każdego komputera podaj hasło **tylko ten pierwszy raz**, aby skopiować klucz:
   \`\`\`bash
   ssh-copy-id login@stacja_robocza_1
   ssh-copy-id login@stacja_robocza_2
   ssh-copy-id login@stacja_robocza_3
   \`\`\`
   *(Note: jeśli Twoje konto w sieci laboratorium ma globalnie wspóldzielony folder domowy NFS, utworzenie klucza do pliku \`authorized_keys\` na serwerze głównym zazwyczaj automatycznie aktywuje dostęp na wszystkie komputery. Możesz sprawdzić to logując się \`ssh stacja_robocza_1\`).*

## 2. Przygotowanie pliku z hostami (\`machinefile\`)

Narzędzie \`mpirun\` / \`mpiexec\` potrzebuje widzieć na jakie maszyny podzielić tworzenie procesów. Stwórz plik tekstowy o nazwie \`machinefile\`, gdzie każdy wiersz definiuje adres maszyny z opcjonalną ilością slotów procesora:

Zawartość pliku \`machinefile\`:
\`\`\`
localhost slots=2
stacja_robocza_1 slots=4
stacja_robocza_2 slots=4
stacja_robocza_3 slots=4
\`\`\`

## 3. Kompilacja programu

Przed uruchomieniem kodu należy go skompilować. Pamiętaj, aby plik wykonywalny znajdował się na zamontowanym dysku dostępnym **dla wszystkich stacji objętych pracą** oraz by skompilować go dla docelowej architektury (najlepiej używając domyślnego w laboratorium \`Makefile\`).

\`\`\`bash
# Jesli jest dostępny Makefile z plikami projektu:
make main

# Alternatywnie (ręczna kompilacja):
mpicc -O3 -fopenmp mpi+openmp.c -o openmp+mpi -lm
\`\`\`

## 4. Wykonanie programu

Mając pliki z hostami i skompilowany system MPI, wywołaj program decydując o zakresie, nazwie i ilości wątków OpenMP. Wykorzystaj tag \`-f\` (zależnie od wariantu OpenMPI może to być też \`--hostfile\` lub \`-machinefile\`) do wskazania topologii klastra z kroku drugiego.

Składnia Twojego programu: \`./openmp+mpi [górny zakres poszukiwań] [dowolny napis/marker czasowy] [ilosc wątków]\`.

**Pełna komenda odpalająca 10 procesów MPI na wielu maszynach (4 wątki per proces, zakres do 10 mln):**
\`\`\`bash
# Pamiętaj o użyciu zmiennych środowiskowych jeśli wymagane
export OMP_NUM_THREADS=4 

mpiexec -f machinefile -n 10 ./openmp+mpi 10000000 "Start_Cluster" 4
\`\`\`
Wynik wyświetli podsumowanie od roota i czas potrzebny na kooperacyjne wyliczenie operacji.
