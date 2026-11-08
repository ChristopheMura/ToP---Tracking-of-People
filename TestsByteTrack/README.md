# Installation et utilisation de ByteTrack-cpp

Ce projet utilise la bibliothèque C++ suivante pour le suivi multi-personnes :

**ByteTrack-cpp**  
https://github.com/Vertical-Beach/ByteTrack-cpp

ByteTrack est utilisé après le moteur de détection YOLO. YOLO fournit les positions des personnes et leurs scores de confiance, puis ByteTrack associe ces détections entre les différentes images afin d'attribuer et conserver un identifiant pour chaque personne.

---

## 1. Installation des dépendances

Le projet nécessite notamment :

- un compilateur compatible C++17 ;
- CMake ;
- Eigen ;
- Boost ;
- GoogleTest pour les tests fournis avec ByteTrack-cpp.

Sur Ubuntu/Debian :

```bash
sudo apt update

sudo apt install \
    build-essential \
    cmake \
    libeigen3-dev \
    libboost-all-dev \
    libgtest-dev
```

La version de CMake peut être vérifiée avec :

```bash
cmake --version
```

---

## 2. Récupération de ByteTrack-cpp

Depuis le dossier `ProjetVision` :

```bash
git clone https://github.com/Vertical-Beach/ByteTrack-cpp.git
```

L'organisation utilisée est la suivante :

```text
ProjetVision/
├── ByteTrack-cpp/
│   ├── include/
│   ├── src/
│   └── ...
│
└── ToP---Tracking-of-People/
    └── TestsByteTrack/
```

---

## 3. Compilation de ByteTrack-cpp

Entrer dans le dépôt :

```bash
cd ByteTrack-cpp
```

Créer un dossier de compilation :

```bash
mkdir build
cd build
```

Configurer le projet :

```bash
cmake ..
```

Puis compiler :

```bash
make
```

La bibliothèque dynamique obtenue se trouve dans :

```text
ByteTrack-cpp/build/libbytetrack.so
```

La présence d'une ligne similaire à :

```text
Built target bytetrack
```

indique que la bibliothèque ByteTrack a été compilée.

---

## 4. Compilation des tests internes de ByteTrack

Pour compiler également les tests fournis avec la bibliothèque :

```bash
cmake .. -DBUILD_BYTETRACK_TEST=ON
make
```

Puis lancer les tests :

```bash
ctest --verbose
```

Ces tests permettent de vérifier que la bibliothèque ByteTrack fonctionne correctement indépendamment de notre application.

---

## 5. Organisation des tests du projet

Les tests réalisés pour comprendre et valider ByteTrack sont placés dans le dépôt du projet :

```text
ProjetVision/
├── ByteTrack-cpp/
│   ├── include/
│   ├── src/
│   └── build/
│       └── libbytetrack.so
│
└── ToP---Tracking-of-People/
    └── TestsByteTrack/
        ├── Test1.cpp
        ├── Test2.cpp
        ├── Test3.cpp
        ├── CMakeLists.txt
        └── build/
```

Les dossiers ont les rôles suivants :

```text
ByteTrack-cpp/
→ bibliothèque externe ByteTrack

TestsByteTrack/
→ programmes développés pour tester son fonctionnement

TestsByteTrack/build/
→ fichiers générés lors de la compilation des tests
```

---

## 6. Inclusion de ByteTrack dans un programme

Dans un fichier C++, ByteTrack est inclus avec :

```cpp
#include "ByteTrack/BYTETracker.h"
```

Les chemins vers les fichiers de la bibliothèque sont fournis au compilateur avec CMake.

Il n'est donc pas nécessaire d'utiliser un chemin absolu comme :

```cpp
#include "/home/user/.../ByteTrack/BYTETracker.h"
```

---

## 7. Compilation des tests

Se placer dans :

```bash
cd ToP---Tracking-of-People/TestsByteTrack
```

Créer le dossier de compilation :

```bash
mkdir build
cd build
```

Configurer le projet :

```bash
cmake ..
```

Puis compiler :

```bash
make
```

Cela compile tous les tests déclarés dans le `CMakeLists.txt`.

Les exécutables générés peuvent ensuite être lancés :

```bash
./Test1
```

```bash
./Test2
```

```bash
./Test3
```

Pour compiler uniquement un test :

```bash
make Test1
```

ou :

```bash
make Test2
```

ou :

```bash
make Test3
```

---

## 8. Nettoyer et reconstruire le projet

Si le projet ou le dossier `TestsByteTrack` est déplacé, il est nécessaire de supprimer le cache CMake existant.

Depuis `TestsByteTrack` :

```bash
rm -rf build
mkdir build
cd build
cmake ..
make
```

Cela permet à CMake de reconstruire ses fichiers avec les nouveaux chemins.

---

## 9. Tests actuellement réalisés

Les tests ByteTrack suivent une progression simple.

### Test 1 — Une personne

Une personne artificielle se déplace de quelques pixels à chaque image.

Objectif :

```text
vérifier que ByteTrack conserve le même ID
```

---

### Test 2 — Deux personnes

Deux personnes artificielles se déplacent simultanément.

Objectif :

```text
vérifier que ByteTrack crée et conserve
deux pistes différentes
```

---

### Test 3 — Occlusion temporaire

Deux personnes sont suivies, mais l'une des deux disparaît volontairement des détections pendant plusieurs images.

Objectif :

```text
vérifier que ByteTrack peut conserver une piste
pendant une courte absence de détection
```

Ces tests permettent de comprendre le fonctionnement de ByteTrack avant son intégration avec les véritables détections YOLO.

---

## 10. Procédure complète

Installation des dépendances :

```bash
sudo apt update

sudo apt install \
    build-essential \
    cmake \
    libeigen3-dev \
    libboost-all-dev \
    libgtest-dev
```

Récupération et compilation de ByteTrack :

```bash
cd ProjetVision

git clone https://github.com/Vertical-Beach/ByteTrack-cpp.git

cd ByteTrack-cpp

mkdir build
cd build

cmake ..
make
```

Compilation des tests du projet :

```bash
cd ../../ToP---Tracking-of-People/TestsByteTrack

mkdir build
cd build

cmake ..
make
```

Exécution :

```bash
./Test1
./Test2
./Test3
```