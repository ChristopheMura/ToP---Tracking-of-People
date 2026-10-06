// *************************************************************************************
// Détection YOLO26 (export ONNX end2end, sortie 1x300x6) avec OpenCV DNN
// Usage : ./yolo_detect <modele.onnx> <image | video | index_camera> [taille_entree]
//
// Principe général :
//   1. Charger le réseau ONNX avec OpenCV DNN.
//   2. Pour chaque image : letterbox -> blob NCHW normalisé -> forward().
//   3. Lire les 300 détections déjà triées par le modèle (YOLO26 est "NMS-free" :
//      la suppression des doublons est faite DANS le réseau, d'où l'absence de NMSBoxes).
//   4. Filtrer (confiance + classe), remettre les boîtes à l'échelle de l'image d'origine.
//   5. Dessiner et afficher.
// *************************************************************************************

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/dnn.hpp>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>


// Une détection = une boîte (en pixels de l'image d'ORIGINE), un score, un identifiant de classe.
struct Detection
{
	cv::Rect box;
	float conf;
	int classId;
};


// Seuil de confiance minimal pour garder une détection.
constexpr float CONF_THRESHOLD 	= 0.60f;
// Si true, on ne garde que la classe 0 ("person" dans COCO).
constexpr bool PERSON_ONLY		= true;


// *************************************************************************************
// letterbox : redimensionne l'image en conservant son ratio, puis la colle dans un carré
// size x size rempli de gris (114 = valeur de padding utilisée par Ultralytics).
//
// - scale (sortie) : facteur appliqué à l'image. Il sert ensuite à ramener les boîtes
//   prédites dans le repère de l'image d'origine (division par scale).
// - L'image redimensionnée est collée en HAUT À GAUCHE (Rect(0,0,w,h)). 
// 	 Conséquence : aucun offset à retirer lors de la remise à l'échelle, seulement une division.
// *************************************************************************************
static cv::Mat letterbox(const cv::Mat& img, int size, float& scale)
{
	// Le plus petit des deux ratios garantit que l'image tient entièrement dans le carré.
	scale = std::min(size / static_cast<float>(img.cols), size / static_cast<float>(img.rows));
	const int w = static_cast<int>(std::round(img.cols * scale));
	const int h = static_cast<int>(std::round(img.rows * scale));

	cv::Mat resized;
	cv::resize(img, resized, cv::Size(w, h));

	// Canevas gris de taille size x size, même type que l'entrée (CV_8UC3 attendu).
	cv::Mat img_out(size, size, img.type(), cv::Scalar(114, 114, 114));
	resized.copyTo(img_out(cv::Rect(0, 0, w, h)));
	
	return img_out;
}


// *************************************************************************************
// detect : prétraitement + inférence + post-traitement pour UNE image.
// Retourne les détections filtrées, en coordonnées de l'image d'origine.
// *************************************************************************************
static std::vector<Detection> detect(cv::dnn::Net& net, const cv::Mat& frame, int inputSize)
{
	float scale = 1.0f;
	const cv::Mat input = letterbox(frame, inputSize, scale);

	// blobFromImage fait trois choses d'un coup :
	//   - multiplie par 1/255 -> pixels dans [0,1] (normalisation attendue par YOLO)
	//   - swapRB = true      -> BGR (OpenCV) vers RGB (ce que le modèle a vu à l'entraînement)
	//   - réorganise en NCHW -> tenseur 1x3xHxW
	const cv::Mat blob = cv::dnn::blobFromImage(input, 1.0 / 255.0, cv::Size(inputSize, inputSize), cv::Scalar(), true, false);
	net.setInput(blob);
	const cv::Mat out = net.forward();	// l'inférence proprement du modèle

	// Garde-fou : on vérifie qu'on a bien un export end2end (1 x N x 6).
	if (out.dims != 3 || out.size[2] != 6)
        throw std::runtime_error("Sortie inattendue : ce code attend un export end2end (1x300x6)");

	const int n = out.size[1];				// nombre de détections
    const float* data = out.ptr<float>();	// accès brut : suppose un Mat continu en float32
    const cv::Rect frameRect(0, 0, frame.cols, frame.rows);

	std::vector<Detection> dets;
    for (int i = 0; i < n; ++i)
	{
		// Chaque ligne = 6 floats. Coordonnées exprimées dans le repère de l'image letterboxée.
        const float* r = data + i * 6;           // x1, y1, x2, y2, conf, classe
        const float conf = r[4];
        const int cls = static_cast<int>(r[5]);

		// Filtrage : score insuffisant ou mauvaise classe -> on jette.
        if (conf < CONF_THRESHOLD) continue;
        if (PERSON_ONLY && cls != 0) continue;

		// Retour au repère de l'image d'origine : division par scale (pas d'offset, cf. letterbox).
		// ⚠ static_cast<int> tronque au lieu d'arrondir -> erreur systématique d'un pixel max,
		//   vers le haut/gauche. Négligeable pour l'affichage, à corriger (std::lround) si ces
		//   boîtes alimentent un tracker ou une mesure.
        cv::Rect box(cv::Point(static_cast<int>(r[0] / scale), static_cast<int>(r[1] / scale)),
                     cv::Point(static_cast<int>(r[2] / scale), static_cast<int>(r[3] / scale)));
		
		// Intersection avec le cadre de l'image : supprime les débordements, notamment
		// les boîtes qui tombent dans la zone de padding grise.
        box &= frameRect;
        if (box.area() > 0) dets.push_back({box, conf, cls});
    }

	return dets;
}


// *************************************************************************************
// draw : dessine les boîtes, leurs étiquettes et le temps de traitement sur l'image.
// *************************************************************************************
static void draw(cv::Mat& frame, const std::vector<Detection>& dets, double ms)
{
    for (const auto& d : dets)
	{
		cv::rectangle(frame, d.box, cv::Scalar(0, 255, 0), 3);

		// ⚠ Libellé "personne" codé en dur : si tu passes PERSON_ONLY à false, TOUTES les
		//   classes seront affichées comme "personne". d.classId n'est jamais utilisé ici.
		const std::string label = cv::format("personne %.0f%%", d.conf * 100.0f);
		int baseline = 0;
		const cv::Size ts = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.8, 2, &baseline);

		// Si la boîte touche le haut de l'image, on garde le texte visible
		const int y = std::max(d.box.y, ts.height + baseline);

		// Fond plein derrière le texte pour la lisibilité, puis le texte en noir.
		cv::rectangle(frame,
					cv::Point(d.box.x, y - ts.height - baseline),
					cv::Point(d.box.x + ts.width, y),
					cv::Scalar(0, 255, 0), cv::FILLED);

		// ⚠ Ce "FPS" = 1000 / temps de detect() uniquement. Il ignore la capture, draw(),
		//   imshow() et waitKey(). Le FPS réel de la boucle est donc PLUS BAS que celui affiché.
		cv::putText(frame, label, cv::Point(d.box.x, y - baseline),
					cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
	}
    cv::putText(frame, cv::format("%.1f ms (%.1f FPS)", ms, 1000.0 / ms),
                cv::Point(10, 25), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(0, 0, 255), 2);
}



int main(int argc, char** argv)
{
	if (argc < 3)
	{
		fprintf(stderr, "Usage : %s <modele.onnx> <image | video | index_camera> [taille_entree=640]\n", argv[0]);
		exit(1);
	}
	// Recupération du chemin du model ONNX
	const std::string modelPath = argv[1];
	// Recuperation de la source 0 pour la webcam ou bien entrer le chemin d'une image ou d'une video
	const std::string source = argv[2];

	// Taille d'entrée du RÉSEAU (côté du carré letterbox), pas celle de l'image/vidéo source.
	// Elle doit correspondre à l'imgsz utilisé lors de l'export ONNX (sauf export dynamique).
	// ⚠ std::stoi lève une exception non rattrapée si argv[3] n'est pas un nombre -> crash.
	int inputSize;
	
	if (argc > 3)
		inputSize = std::stoi(argv[3]);
	else
		inputSize = 640;

	std::cout << "OpenCV " << CV_VERSION << std::endl;

	/* ---------- Chargement du modèle ---------- */
	cv::dnn::Net net;
    try
	{
		// Lecture du modèle ONNX
        net = cv::dnn::readNetFromONNX(modelPath);
    }
	catch (const cv::Exception& e)
	{
        std::cerr << "Chargement du modèle impossible :\n" << e.what() << "\n";
        return 1;
    }

	// Utilisation de l'implémentation interne d'OpenCV dnn plutot qu'une implémentation externe tel que CUDA
	net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);

	// Préchauffage / validation
	try
	{
		// On test le modèle avec une image en niveau de gris
		const cv::Mat dummy(inputSize, inputSize, CV_8UC3, cv::Scalar(114, 114, 114));
		detect(net, dummy, inputSize);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Le modèle n'accepte pas une entrée de " << inputSize << "x" << inputSize
				<< ".\nUtilise la taille utilisée à l'export (voir l'entrée 'images' sur netron.app).\n"
				<< "Détail : " << e.what() << "\n";
		return 1;
	}

	using clock = std::chrono::steady_clock;

	/* ---------- Cas 1 : image fixe ---------- */
	// Stratégie : on tente d'abord imread. Si ça réussit, c'est une image ; sinon on bascule
	// sur VideoCapture. Simple, mais imread sur un fichier vidéo peut émettre des warnings.
	cv::Mat img = cv::imread(source);
	if (!img.empty())
	{
		/* --- Calcul du temps de traitement --- */
        const auto t0 = clock::now();
        const auto dets = detect(net, img, inputSize);
        const double ms = std::chrono::duration<double, std::milli>(clock::now() - t0).count();
		/* ------------------------------------- */
		
        std::cout << dets.size() << " détection(s), " << ms << " ms\n";

        draw(img, dets, ms);

        cv::imshow("YOLO", img);
        cv::waitKey(0);

        return 0;
    }
	/* ---------- Cas 2 : vidéo ou caméra ---------- */
	cv::VideoCapture cap;
	
	const bool isIndex = !source.empty() && std::all_of(source.begin(), source.end(), ::isdigit);
	if (isIndex)
		cap.open(std::stoi(source));
	else
		cap.open(source);
	
	if (!cap.isOpened())
	{
        std::cerr << "Impossible d'ouvrir la source : " << source << "\n";
        return 1;
    }

	cv::Mat frame;

	// Boucle principale : s'arrête à la fin de la vidéo, sur une erreur de lecture caméra
	// (silencieusement — aucun message), ou sur Échap.
	while (cap.read(frame))
	{
		/* --- Calcul du temps de traitement --- */
		const auto t0 = clock::now();
		const auto dets = detect(net, frame, inputSize);
		const double ms = std::chrono::duration<double, std::milli>(clock::now() - t0).count();
		/* ------------------------------------- */

		draw(frame, dets, ms);

		cv::imshow("YOLO", frame);
		if (cv::waitKey(1) == 27) break;
	}

	return 0;
}