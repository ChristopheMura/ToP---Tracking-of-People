#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include <iostream>
#include <vector>
#include <cmath>

#include "ByteTrack/BYTETracker.h"

#include "../include/mylib.h"

// Seuil de confiance minimal pour garder une détection.
#define ACC_THRESHOLD 0.10f     // seuil de 60%
// Si true, on ne garde que la classe 0 ("person" dans COCO).
#define PERSON_ONLY false

#define CHOIX 0

// Une détection = une boîte (en pixels de l'image d'ORIGINE), un score, un identifiant de classe.
struct Detection
{
    cv::Rect box;
    float acc;
    int classId;
};


static cv::Mat letterbox(const cv::Mat& img, int size, float& scale);

static std::vector<Detection> detect(cv::dnn::Net& net, const cv::Mat& frame, int inputSize);

static void drawBox(cv::Mat& frame, const std::vector<Detection>& dets);

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage : %s <modele.onnx> <image | video | index_camera> [taille_entree=640]\n", argv[0]);
        exit(0);
    }

    // Recupération du chemin du model ONNX
	const std::string modelPath = *++argv;
	// Recuperation de la source 0 pour la webcam ou bien entrer le chemin d'une image ou d'une video
	const std::string source = *++argv;

    // Taille d'entrée du RÉSEAU
    int inputSize;
    if (argc > 3)
        inputSize = std::stoi(*++argv);
    else
        inputSize = 640;
    
    cv::dnn::Net network;
    try
    {
        // Lecture du modele ONNX
        network = cv::dnn::readNetFromONNX(modelPath);
    }
    catch(const std::exception& e)
    {
        std::cerr << "Chargement du modele impossible" << e.what() << '\n';
        exit(0);
    }
    
    // Utilisation de l'implémentation interne d'OpenCV dnn plutot qu'une implémentation externe tel que CUDA
	network.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);

    // Préchauffage / validation
	try
	{
		// On test le modèle avec une image en niveau de gris
		const cv::Mat dummy(inputSize, inputSize, CV_8UC3, cv::Scalar(114, 114, 114));
		//detect(net, dummy, inputSize);
	}
	catch (const std::exception& e)
	{
		std::cerr << "Le modèle n'accepte pas une entrée de " << inputSize << "x" << inputSize
				<< ".\nUtilise la taille utilisée à l'export (voir l'entrée 'images' sur netron.app).\n"
				<< "Détail : " << e.what() << "\n";
		exit(0);
	}

    /* ------------- Cas 1 : image fixe ------------ */
    cv::Mat img = cv::imread(source);
    if (!img.empty())
    {
        auto dets = detect(network, img, inputSize);
        drawBox(img, dets);

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
        exit(0);
    }

    cv::Mat frame;

    /*--------------------
    Instanciation de ByteTrack 
    -----------------------*/
    byte_track::BYTETracker tracker(
        30,     // FPS
        30,     // track buffer
        0.5f,   // track threshold
        0.6f,   // high threshold
        0.8f    // match threshold
    );

    while (cap.read(frame))
    {
        auto dets = detect(network, frame, inputSize);
        //Tableau de detection donné a bytetrack
        std::vector<byte_track::Object> objects;
        drawBox(frame, dets);

#if 1 == CHOIX
        cv::Mat img_gray = detectionContour(frame);

        cv::imshow("YOLO", img_gray);
#elif 2 == CHOIX
        cv::Mat img_seuil = seuillage_couleur(frame);

        cv::imshow("YOLO", img_seuil);
#else
        for(const auto &det : dets)
        {
            if(det.classId != 0)
                continue;

            else
            {
                objects.emplace_back(
                    byte_track::Rect<float>(
                    det.box.x,
                    det.box.y,
                    det.box.width,
                    det.box.height
                )
                det.classId,
                det.acc
                )
            }
        }
        cv::imshow("YOLO", frame);
#endif
        if (cv::waitKey(1) == 27) break;
    }

    return 0;
}

// *************************************************************************************
// letterbox : redimensionne l'image en conservant son ratio, puis la colle dans un carré
// size x size rempli de gris (114 = valeur de padding utilisée par Ultralytics).
//
// - scale (sortie) : facteur appliqué à l'image. Il sert ensuite à ramener les boîtes
//   prédites dans le repère de l'image d'origine.
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

    // blobFromImage fait trois choses :
	//   - multiplie par 1/255 -> pixels dans [0,1] (normalisation attendue par YOLO)
	//   - swapRB = true      -> BGR (OpenCV) vers RGB (ce que le modèle a vu à l'entraînement)
	//   - réorganise en NCHW -> tenseur 1x3xHxW
    const cv::Mat blob = cv::dnn::blobFromImage(input, 1.0 / 255.0, cv::Size(inputSize, inputSize), cv::Scalar(), true, false);
	net.setInput(blob);
	const cv::Mat out = net.forward();	// l'inférence du modèle
    // out est un tenseur de dimension 3 et de taille 1x300x6
    // le modèle yolo retourne les 300 meilleurs détection
    // le 6 correspend aux données de chaque détection
    // x1, y1, x2, y2, accuracy, classeId

    // std::cout << "dims=" << out.dims << " shape=";
    // for (int d = 0; d < out.dims; ++d) std::cout << out.size[d] << (d + 1 < out.dims ? "x" : "\n");
    // std::cout << "type=" << out.type() << " (CV_32F=" << CV_32F << ")\n";

    // Garde-fou : on vérifie qu'on a bien un export end2end (1 x N x 6).
	if (out.dims != 3 || out.size[2] != 6)
        throw std::runtime_error("Sortie inattendue : ce code attend un export end2end (1x300x6)");
    
    const int n = out.size[1];  // nb de detection
    const float* data = out.ptr<float>();
    const cv::Rect frameRect(0, 0, frame.cols, frame.rows);

    std::vector<Detection> dets;
    for (int i = 0; i < n; i++)
    {
        // Chaque ligne = 6 floats. Coordonnées exprimées dans le repère de l'image letterboxée.
        const float* r = data + i * 6;              // x1, y1, x2, y2, conf, classe
        const float acc = r[4];                     // On recupère l'accuracy
        const int classId = static_cast<int>(r[5]); // On recupère la classId

        // Filtrage : score insuffisant ou mauvaise classe -> on jette.
        if (acc < ACC_THRESHOLD) continue;
        //if (PERSON_ONLY && (classId != 0)) continue;

        // On crée une box avec les coordonnées des detections
        cv::Rect box(cv::Point(static_cast<int>(r[0]/scale),    // x1
                            static_cast<int>(r[1]/scale)),      // y1
                     cv::Point(static_cast<int>(r[2]/scale),    // x2
                            static_cast<int>(r[3]/scale)));     // y2

        // Intersection avec le cadre de l'image : supprime les débordements, notamment
		// les boîtes qui tombent dans la zone de padding grise.
        box &= frameRect;
        if (box.area() > 0)
            dets.push_back({box, acc, classId});
    }

    return dets;
}

// *************************************************************************************
// draw : dessine les boîtes et leurs étiquettes.
// *************************************************************************************
static void drawBox(cv::Mat& frame, const std::vector<Detection>& dets)
{
    std::map<int, char*> my_map = {
        {0, "person"},
        {1, "bicycle"},
        {2, "car"},
        {3, "motorcycle"},
        {4, "airplane"},
        {5, "bus"},
        {6, "train"},
        {7, "truck"},
        {8, "boat"},
        {9, "traffic light"},
        {10, "fire hydrant"},
        {11, "stop sign"},
        {12, "parking meter"},
        {13, "bench"},
        {14, "bird"},
        {15, "cat"},
        {16, "dog"},
        {17, "horse"},
        {18, "sheep"},
        {19, "cow"},
        {20, "elephant"},
        {21, "bear"},
        {22, "zebra"},
        {23, "giraffe"},
        {24, "backpack"},
        {25, "umbrella"},
        {26, "handbag"},
        {27, "tie"},
        {28, "suitcase"},
        {29, "frisbee"},
        {30, "skis"},
        {31, "snowboard"},
        {32, "sports ball"},
        {33, "kite"},
        {34, "baseball bat"},
        {35, "baseball glove"},
        {36, "skateboard"},
        {37, "surfboard"},
        {38, "tennis racket"},
        {39, "bottle"},
        {40, "wine glass"},
        {41, "cup"},
        {42, "fork"},
        {43, "knife"},
        {44, "spoon"},
        {45, "bowl"},
        {46, "banana"},
        {47, "apple"},
        {48, "sandwich"},
        {49, "orange"},
        {50, "broccoli"},
        {51, "carrot"},
        {52, "hot dog"},
        {53, "pizza"},
        {54, "donut"},
        {55, "cake"},
        {56, "chair"},
        {57, "couch"},
        {58, "potted plant"},
        {59, "bed"},
        {60, "dining table"},
        {61, "toilet"},
        {62, "tv"},
        {63, "laptop"},
        {64, "mouse"},
        {65, "remote"},
        {66, "keyboard"},
        {67, "cell phone"},
        {68, "microwave"},
        {69, "oven"},
        {70, "toaster"},
        {71, "sink"},
        {72, "refrigerator"},
        {73, "book"},
        {74, "clock"},
        {75, "vase"},
        {76, "scissors"},
        {77, "teddy bear"},
        {78, "hair drier"},
        {79, "toothbrush"}
    };

    for (const auto& detection : dets)
    {
        cv::rectangle(frame, detection.box, cv::Scalar(0, 255, 0), 3);
        // Libellé "personne" codé en dur
        const std::string label = cv::format("%s %.0f%%", my_map[detection.classId], detection.acc * 100.0f);
        int baseline = 0;
        const cv::Size ts = cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.8, 2, &baseline);

        // Fond plein derrière le texte pour la lisibilité, puis le texte en noir
        cv::rectangle(frame,
					cv::Point(detection.box.x, detection.box.y - ts.height),
					cv::Point(detection.box.x + ts.width, detection.box.y),
					cv::Scalar(0, 255, 0), cv::FILLED);
        
        cv::putText(frame, label, cv::Point(detection.box.x, detection.box.y),
					cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(0, 0, 0), 2);
    }
}