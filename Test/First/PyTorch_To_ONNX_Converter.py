from ultralytics import YOLO

# Charger le modèle YOLO26 Nano
model = YOLO("yolo26n.pt")

# L'argument end2end=True intègre la sélection unique directement dans le fichier ONNX
model.export(format="onnx", imgsz=640, end2end=True)
# Fichier généré : yolo26n.onnx