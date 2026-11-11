# ToP---Tracking-of-People
ToP - Tracking of People
Lucas c'est pas le Pinguin qui glisse le plus loin
Quristauffe n'est pas la souris la plus verte

# Tracking of People

Projet de vision embarquée visant à détecter et suivre plusieurs personnes à partir d'une caméra fixe.

Le système repose sur :

- **OpenCV** pour l'acquisition vidéo et l'inférence ;
- **YOLO26** exporté au format ONNX pour la détection ;
- **ByteTrack-cpp** pour associer les détections entre les images et attribuer un ID à chaque personne.

## Fonctionnement

YOLO détecte les objets présents dans chaque image. Les détections correspondant à la classe `person` sont ensuite transmises à ByteTrack.

ByteTrack permet de conserver un identifiant pour chaque personne au fil des images.

## Dépendances

Le projet nécessite :

- C++17
- CMake
- OpenCV 5
- Eigen3
- ByteTrack-cpp

Sur Ubuntu/Debian :

```bash
sudo apt update

sudo apt install \
    build-essential \
    cmake \
    libeigen3-dev 
```

## Récupération de ByteTrack-cpp

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
    └── Detection
```

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

---
## Organisation

```text
Detection/code/
├── src/
│   ├── main.cpp
│   └── mylib.cpp
├── include/
│   └── mylib.h
├── CMakeLists.txt
└── build/
```

Le dossier `build/` contient uniquement les fichiers générés lors de la compilation et ne doit pas être versionné.

## Compilation du projet ToP

Depuis le dossier contenant le `CMakeLists.txt` :

```bash
mkdir build
cd build

cmake .. -DCMAKE_PREFIX_PATH=[chemin-vers_opencv5]
make
```

L'exécutable généré est :

```bash
./tracking_of_people
```

## Utilisation

Le programme attend :

```text
tracking_of_people <modele.onnx> <image | video | index_camera> [taille_entree]
```

Exemple avec une vidéo :

```bash
./tracking_of_people ./models/PersonDetection.onnx video.mp4
```

Exemple avec la webcam :

```bash
./tracking_of_people ./models/PersonDetection.onnx 0
```

La taille d'entrée par défaut du réseau est `640`.
