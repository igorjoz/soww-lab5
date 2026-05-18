# Skrócona instrukcja uruchamiania na klastrze w laboratorium (ściąga)

**1. Zalogowanie i dostęp bezhasłowy (tylko za pierwszym razem!)**
Zaloguj się na maszynę główną (master) w laboratorium. Wygeneruj klucz i prześlij go na inne komputery, by MPI mogło uruchamiać się bez pytania o hasło:
\`\`\`bash
ssh-keygen -t ed25519   # (klikaj cały czas Enter)
ssh-copy-id login@nazwa_maszyny_1
ssh-copy-id login@nazwa_maszyny_2
\`\`\`

**2. Plik z maszynami (\`machinefile\`)**
W folderze z Twoim kodem stwórz plik o nazwie \`machinefile\` posiadający listę komputerów (węzłów), które mają liczyć.
*Przykład:*
\`\`\`
nazwa_maszyny_1 slots=4
nazwa_maszyny_2 slots=4
\`\`\`

**3. Kompilacja programu**
Upewnij się, że jesteś we współdzielonym folderze (NFS) widocznym dla innych maszyn i skompiluj projekt za pomocą przygotowanego \`Makefile\`:
\`\`\`bash
make main
\`\`\`

**4. Szybki start (Uruchomienie)**
Odpal z użyciem argumentu \`-f machinefile\`. Program przyjmuje argumenty w kolejności: \`[limit] [nazwa] [liczba_watkow_openmp]\`.
\`\`\`bash
# Odpalenie na 8 procesach (które rozejdą się na maszyny wg. machinefile), po 4 wątki OpenMP każdy:
mpiexec -f machinefile -n 8 ./openmp+mpi 10000000 "TestKlaster" 4
\`\`\`